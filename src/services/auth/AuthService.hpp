#pragma once

#include "services/common/DataBaseAccessable.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_response_cookie.hpp>

namespace services::auth
{

class AuthService final : public common::DataBaseAccessable,
                          public userver::server::handlers::HttpHandlerJsonBase
{
public:
  static constexpr std::string_view kName = "auth-service";

  static constexpr std::string_view authSchema = "Bearer";

  explicit AuthService (
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &component_context);

  bool
  checkLogin () const
  {
    return true;
  }

  std::optional<std::string> getAccessToken (std::string_view login) const;

  std::optional<std::string> getCookieAccessToken (const HttpRequest &) const;
  void updateAccessToken (std::string_view login,
                          std::string_view token) const;

  bool validateCredentials (std::string_view login,
                            std::string_view password) const;

  bool validateCookie (const HttpRequest &request) const;

  Value HandleRequestJsonThrow (const HttpRequest &request,
                                const Value &request_json,
                                RequestContext &) const override;

  Value HandleLoginRequestJsonThrow (const HttpRequest &request,
                                     const Value &request_json,
                                     RequestContext &) const;

  Value HandleLogoutRequestJsonThrow (const HttpRequest &request,
                                      const Value &request_json,
                                      RequestContext &) const;
};
}
