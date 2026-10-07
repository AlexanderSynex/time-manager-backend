#pragma once

#include "info/Worker.hpp"
#include "services/common/DataBaseAccessable.hpp"

#include <string_view>

#include <userver/clients/dns/component.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/component_list.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>
#include <userver/testsuite/testsuite_support.hpp>

namespace services::control_role
{

///@brief Служебный сервис для управления данными о пользователях
class AdministrationService final
    : public common::DataBaseAccessable,
      public userver::server::handlers::HttpHandlerJsonBase
{
public:
  static constexpr std::string_view kName = "administration-service";

  explicit AdministrationService (
      const userver::components::ComponentConfig &config,
      const userver::components::ComponentContext &component_context);

  Value HandleRequestJsonThrow (const HttpRequest &request,
                                const Value &request_json,
                                RequestContext &context) const override;

private:
  Value HandleUserJsonThrow (const HttpRequest &request,
                             const Value &request_json,
                             RequestContext &context) const;
  userver::formats::json::Value modifyUser (Worker &&user,
                                            const Value &request_json) const;
  void modifyUser (const Worker &user, Worker::Info &&info) const;
  void modifyUserAccount (const Worker &user,
                          std::string_view raw_password) const;
  void modifyUserInfo (const Worker &user, Worker::Info &&info) const;

private:
  userver::storages::postgres::ClusterPtr p_db = nullptr;
};

} // namespace services::control_role
