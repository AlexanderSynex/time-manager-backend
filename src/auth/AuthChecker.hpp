#pragma once

#include "auth/Cache.hpp"
#include "auth/Info.hpp"

#include "env/cache/Cookies.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <userver/http/common_headers.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/auth/user_auth_info.hpp>
#include <userver/server/auth/user_scopes.hpp>
#include <userver/server/handlers/auth/auth_checker_base.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/request/request_context.hpp>

#include <vector>

namespace services::auth::schemas
{
template <typename AuthSchema>
class AuthChecker final
    : public userver::server::handlers::auth::AuthCheckerBase
{
public:
  using AuthCheckResult = userver::server::handlers::auth::AuthCheckResult;

  AuthChecker (const services::auth::cache::AuthCache &auth_cache,
               std::vector<userver::server::auth::UserScope> required_scopes)
      : auth_cache_ (auth_cache), required_scopes_{ required_scopes }
  {
  }

  std::optional<std::string>
  extractToken (const userver::server::http::HttpRequest &request) const
  {
    if (request.HasCookie (env::cache::cookies::token.data ()))
      {
        return request.GetCookie (env::cache::cookies::token.data ());
      }

    auto authHeader
        = request.GetHeader (userver::http::headers::kAuthorization);
    if (authHeader.empty ())
      {
        return std::nullopt;
      }

    auto token = AuthSchema::extractToken (authHeader);

    const auto authSchemaPos = authHeader.find (' ');

    return std::string (authHeader.data () + authSchemaPos + 1);
  }

  [[nodiscard]] userver::server::handlers::auth::AuthCheckResult
  CheckAuth (
      const userver::server::http::HttpRequest &request,
      userver::server::request::RequestContext &request_context) const override
  {
    if (request.GetMethod () == userver::server::http::HttpMethod::kOptions)
      {
        return {};
      }
    auto token = extractToken (request);
    if (not token.has_value ())
      {
        return userver::server::handlers::auth::AuthCheckResult{
          userver::server::handlers::auth::AuthCheckResult::Status::
              kTokenNotFound,
          {},
          "Invalid header format",
          userver::server::handlers::HandlerErrorCode::kUnauthorized
        };
      }
    const auto snapshot = auth_cache_.Get ();
    auto it = snapshot->find (
        userver::server::auth::UserAuthInfo::Ticket{ token.value () });
    if (it == snapshot->end ())
      {
        return AuthCheckResult{ AuthCheckResult::Status::kForbidden,
                                "Token invalid" };
      }

    const services::auth::AuthInfo &info = it->second;

    for (auto scope : required_scopes_)
      {
        if (std::find (info.scopes.begin (), info.scopes.end (), scope)
            == info.scopes.end ())
          {
            return AuthCheckResult{ AuthCheckResult::Status::kForbidden,
                                    fmt::format ("No scope '{}' permission",
                                                 scope.GetValue ()) };
          }
      }
    request_context.SetData ("table_id", info.table_id);
    return {};
  }

  [[nodiscard]] bool
  SupportsUserAuth () const noexcept override
  {
    return true;
  }

private:
  const services::auth::cache::AuthCache &auth_cache_;
  const std::vector<userver::server::auth::UserScope> required_scopes_;
};

}
