#pragma once

#include "auth/Cache.hpp"

#include <string_view>

#include <userver/server/handlers/auth/auth_checker_factory.hpp>

namespace services::auth
{

class AuthCheckerFactory final
    : public userver::server::handlers::auth::AuthCheckerFactoryBase
{
public:
  static constexpr std::string_view kAuthType = "bearer";

  explicit AuthCheckerFactory (
      const userver::components::ComponentContext &context);

  userver::server::handlers::auth::AuthCheckerBasePtr MakeAuthChecker (
      const userver::server::handlers::auth::HandlerAuthConfig &auth_config)
      const override;

private:
  const cache::AuthCache &cache;
};

}
