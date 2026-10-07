#include "services/UserInfoService.hpp"
#include "info/DBInfo.hpp"
#include "services/details/UserService.hpp"

#include <bits/chrono.h>
#include <fmt/format.h>
#include <string>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/yaml/value.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/server/http/http_method.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/io/chrono.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/datetime/date.hpp>

using namespace userver;
using namespace userver::server;
using namespace services::info_role;

UserInfoService::UserInfoService (
    const components::ComponentConfig &config,
    const components::ComponentContext &component_context)
    : control_role::details::UserService (component_context,
                                          db::info::db_name),
      handlers::HttpHandlerJsonBase (config, component_context)
{
}

handlers::HttpHandlerJsonBase::Value
UserInfoService::HandleRequestJsonThrow (const HttpRequest &request,
                                         const Value &request_json,
                                         RequestContext &context) const
{
  if (request.GetMethod () != userver::v3_3_rc::server::http::HttpMethod::kGet)
    {
      throw server::handlers::ClientError (server::handlers::ExternalBody{
          fmt::format ("Unsupported method {}", request.GetMethod ()) });
    }
  auto user = getWorker (context);
  auto builder = formats::json::ValueBuilder{};
  builder["table_id"] = std::to_string (user.value ().id);
  builder["status"] = "нет данных";
  if (request_json.IsEmpty ())
    {
      builder["personal"] = getUserInfo (user.value ());
    }
  return builder.ExtractValue ();
}
