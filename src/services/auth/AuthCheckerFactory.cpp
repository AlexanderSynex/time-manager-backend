#include "AuthCheckerFactory.hpp"

#include "auth/Cache.hpp"
#include "auth/schemas/Bearer.hpp"

#include <fmt/format.h>
#include <memory>
#include <userver/http/common_headers.hpp>
#include <userver/server/auth/user_scopes.hpp>
#include <userver/server/handlers/auth/auth_checker_base.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/request/request_context.hpp>

using namespace services::auth;

AuthCheckerFactory::AuthCheckerFactory (
    const userver::components::ComponentContext &context)
    : cache{ context.FindComponent<cache::AuthCache> () }
{
}

userver::server::handlers::auth::AuthCheckerBasePtr
AuthCheckerFactory::MakeAuthChecker (
    const userver::server::handlers::auth::HandlerAuthConfig &auth_config)
    const
{
  auto scopes
      = auth_config["scopes"].As<userver::server::auth::UserScopes> ({});
  return std::make_shared<schemas::types::AuthBearerChecker> (cache,
                                                       std::move (scopes));
}
