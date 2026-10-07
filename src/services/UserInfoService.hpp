#pragma once

#include "info/Worker.hpp"
#include "services/details/UserService.hpp"
#include <optional>
#include <userver/clients/dns/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include <string_view>
#include <userver/storages/postgres/postgres_fwd.hpp>
#include <userver/storages/query.hpp>
#include <userver/testsuite/testsuite_support.hpp>

namespace services::info_role
{

///@brief Пользовательский сервис для учета рабочего времени пользователей
class UserInfoService final
    : public control_role::details::UserService,
      public userver::server::handlers::HttpHandlerJsonBase
{
public:
  static constexpr std::string_view kName = "user-info-service";

  explicit UserInfoService (
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &component_context);

  Value HandleRequestJsonThrow (const HttpRequest &request,
                                const Value &request_json,
                                RequestContext &) const override;
};
}
