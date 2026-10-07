#include "AdministrationService.hpp"

#include "info/Worker.hpp"
#include "services/details/UserService.hpp"

#include <fmt/format.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/http/status_code.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/cluster_types.hpp>

#include <userver/crypto/hash.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/server/http/http_method.hpp>

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/io/row_types.hpp>
#include <userver/storages/postgres/null.hpp>
#include <userver/storages/postgres/row.hpp>
#include <worktime_postgres_service/sql_queries.hpp>

using namespace userver;
using namespace userver::formats::json;
using namespace services::control_role;

AdministrationService::AdministrationService (
    const components::ComponentConfig &config,
    const components::ComponentContext &component_context)
    : DataBaseAccessable (component_context),
      userver::server::handlers::HttpHandlerJsonBase::HttpHandlerJsonBase (
          config, component_context)
{
}

Value
AdministrationService::HandleRequestJsonThrow (const HttpRequest &request,
                                               const Value &request_json,
                                               RequestContext &context) const
{
  static constexpr auto userTarget = "user";

  std::string target = request.GetPathArg ("target");
  auto handlers = std::unordered_map<std::string, std::function<Value ()>>{
    { userTarget, [&request, &request_json, &context, this] () -> Value
        { return HandleUserJsonThrow (request, request_json, context); } }
  };

  auto handlerIt = handlers.find (target);
  if (handlerIt == handlers.end ())
    {
      throw ClientError (
          ExternalBody{ fmt::format ("Unprocessable target: {}", target) });
    }

  return handlerIt->second ();
}

Value
AdministrationService::HandleUserJsonThrow (const HttpRequest &request,
                                            const Value &request_json,
                                            RequestContext &) const
{
  auto user = getWorker (request_json);
  if (not user.has_value ())
    {
      throw server::handlers::ClientError (
          server::handlers::ExternalBody{ "No user table_id provided" });
    }
  switch (request.GetMethod ())
    {
    case server::http::HttpMethod::kPut:
      {
        return modifyUser (std::move (user.value ()), request_json);
      }
    default:
      throw server::handlers::ClientError (server::handlers::ExternalBody{
          fmt::format ("Unsupported method {}", request.GetMethod ()) });
    }
}

userver::formats::json::Value
AdministrationService::modifyUser (Worker &&user, const Value &json_data) const
{
  if (json_data.GetSize () > 1)
    {
      modifyUser (user, Worker::extractInfo (json_data));
    }
  return getUserInfo (user);
}

void
AdministrationService::modifyUser (const Worker &user,
                                   Worker::Info &&info) const
{
  if (info.password.has_value () or not userExists (user))
    {
      modifyUserAccount (user, info.password.value_or (""));
    }
  modifyUserInfo (user, std::move (info));
}

void
AdministrationService::modifyUserAccount (const Worker &user,
                                          std::string_view raw_password) const
{
  auto newUser = not userExists (user);
  auto trx = db ()->Begin ("account_modification_transaction",
                           storages::postgres::ClusterHostType::kMaster, {});
  auto res = trx.Execute (worktime_postgres_service::sql::kUpdateUser,
                          static_cast<int> (user),
                          userver::crypto::hash::Sha1 (raw_password));
  if (res.RowsAffected ())
    {
      trx.Commit ();
      if (newUser)
        {
          modifyUserInfo (user, {});
        }
    }
  trx.Rollback ();
}

void
AdministrationService::modifyUserInfo (const Worker &user,
                                       Worker::Info &&info) const
{
  auto trx = db ()->Begin ("managing_user_transaction",
                           storages::postgres::ClusterHostType::kMaster, {});
  auto res = trx.Execute (worktime_postgres_service::sql::kUpdateUserInfo,
                          static_cast<int> (user), info.name.value_or (""),
                          info.surname.value_or (""),
                          info.patronymic.value_or (""));
  if (res.RowsAffected ())
    {
      trx.Commit ();
    }
  trx.Rollback ();
}