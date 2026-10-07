#include "AuthService.hpp"

#include "env/cache/Cookies.hpp"

#include <fmt/format.h>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>

#include <userver/formats/json/value.hpp>
#include <userver/http/predefined_header.hpp>
#include <userver/http/status_code.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_response_cookie.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/cluster_types.hpp>

#include "services/common/DataBaseAccessable.hpp"
#include "worktime_postgres_service/sql_queries.hpp"

#include <userver/crypto/hash.hpp>
#include <userver/utils/uuid4.hpp>

#include <userver/formats/json/value_builder.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/http/http_method.hpp>

using namespace userver;
using namespace services::auth;

AuthService::AuthService (
    const userver::components::ComponentConfig &config,
    const userver::components::ComponentContext &component_context)
    : common::DataBaseAccessable (component_context),
      userver::server::handlers::HttpHandlerJsonBase (config,
                                                      component_context)
{
}

bool
AuthService::validateCredentials (std::string_view login,
                                  std::string_view password) const
{
  auto trx = db ()->Begin ("creds_validation_transaction",
                           storages::postgres::ClusterHostType::kMaster, {});
  auto res = trx.Execute (worktime_postgres_service::sql::kValidateCredentials,
                          login, password);
  trx.Rollback ();
  if (res.RowsAffected () > 0)
    return true;
  return false;
}

bool
AuthService::validateCookie (const HttpRequest &request) const
{
  if (not request.HasCookie (env::cache::cookies::token.data ()))
    {
      return false;
    }

  auto trx = db ()->Begin ("auth-token-validation-transaction",
                           storages::postgres::ClusterHostType::kMaster, {});
  auto res
      = trx.Execute (worktime_postgres_service::sql::kValidateToken,
                     request.GetCookie (env::cache::cookies::token.data ()));
  trx.Rollback ();
  return res.RowsAffected ();
}

AuthService::Value
AuthService::HandleRequestJsonThrow (const HttpRequest &request,
                                     const Value &request_json,
                                     RequestContext &context) const
{
  std::string action = request.GetPathArg ("action");
  auto handlers = std::unordered_map<std::string, std::function<Value ()>>{
    { "login",
      [&request, &request_json, &context, this] () -> Value
        {
          return HandleLoginRequestJsonThrow (request, request_json, context);
        } },
    { "logout",
      [&request, &request_json, &context, this] () -> Value
        {
          return HandleLogoutRequestJsonThrow (request, request_json, context);
        } }
  };

  auto handlerIt = handlers.find (action);
  if (handlerIt == handlers.end ())
    {
      throw ClientError (
          ExternalBody{ fmt::format ("Unprocessable action: {}", action) });
    }
  return handlerIt->second ();
}

std::optional<std::string>
AuthService::getAccessToken (std::string_view login) const
{
  auto trx = db ()->Begin ("access_token_transaction",
                           storages::postgres::ClusterHostType::kMaster, {});
  auto res
      = trx.Execute (worktime_postgres_service::sql::kGetAccessToken, login);
  trx.Rollback ();
  return res.AsOptionalSingleRow<std::string> ();
}

std::optional<std::string>
AuthService::getCookieAccessToken (
    const server::http::HttpRequest &request) const
{
  if (not request.HasCookie (env::cache::cookies::token.data ()))
    return std::nullopt;
  return request.GetCookie (env::cache::cookies::token.data ());
}

void
AuthService::updateAccessToken (std::string_view login,
                                std::string_view token) const
{
  auto trx = db ()->Begin ("update_auth_token_transaction",
                           storages::postgres::ClusterHostType::kMaster, {});
  auto res = trx.Execute (worktime_postgres_service::sql::kUpdateAccessToken,
                          login, token);
  trx.Commit ();
}

AuthService::Value
AuthService::HandleLoginRequestJsonThrow (const HttpRequest &request,
                                          const Value &request_json,
                                          RequestContext &) const
{
  if (request.GetMethod ()
      != userver::v3_3_rc::server::http::HttpMethod::kPost)
    {
      throw ClientError (ExternalBody{ "Unsupported method" });
    }

  auto prepareMessage = [] (std::string_view token) -> formats::json::Value
    {
      auto response = formats::json::ValueBuilder{};
      response["token"] = token;
      response["logged"] = true;
      return response.ExtractValue ();
    };

  if (validateCookie (request))
    {
      if (auto token = getCookieAccessToken (request); token.has_value ())
        {
          return prepareMessage (token.value ());
        }
    }

  constexpr auto loginTarget = "login", passwordTarget = "password";
  if (not(request_json.HasMember (loginTarget)
          and request_json.HasMember (passwordTarget)))
    {
      throw ClientError (ExternalBody{ "No login/password provided" });
    }
  if (not request_json[loginTarget].IsString ())
    {
      request.SetResponseStatus (userver::v3_3_rc::http::kBadRequest);
      throw ClientError (ExternalBody{ "Login must be a string" });
    }
  if (not request_json[passwordTarget].IsString ())
    {
      throw ClientError (ExternalBody{ "Password must be a string" });
    }

  auto login = request_json[loginTarget].As<std::string> (),
       password
       = crypto::hash::Sha1 (request_json[passwordTarget].As<std::string> ());

  if (not validateCredentials (login, password))
    {
      request.GetHttpResponse ().SetHeader (http::headers::kWWWAuthenticate,
                                            std::string{ authSchema });
      throw server::handlers::CustomHandlerException (
          server::handlers::HandlerErrorCode::kUnauthorized,
          ExternalBody{ "Wrong login/password" });
    }

  auto token = getAccessToken (login);
  auto access_token = token.value_or (utils::generators::GenerateUuid ());

  if (not token)
    {
      updateAccessToken (login, access_token);
    }

  auto cookie
      = userver::server::http::Cookie{ env::cache::cookies::token.data (),
                                       access_token };
  cookie.SetHttpOnly ();
  cookie.SetPath ("/");
  cookie.SetMaxAge (std::chrono::hours (24));
  request.GetHttpResponse ().SetCookie (std::move (cookie));

  return prepareMessage (access_token);
}

AuthService::Value
AuthService::HandleLogoutRequestJsonThrow (const HttpRequest &request,
                                           const Value &,
                                           RequestContext &) const
{
  if (request.GetMethod () != userver::v3_3_rc::server::http::HttpMethod::kGet)
    {
      throw ClientError (ExternalBody{ "Unsupported method" });
    }
  auto b = formats::json::ValueBuilder{};
  b["action"] = "logout";
  return b.ExtractValue ();
}
