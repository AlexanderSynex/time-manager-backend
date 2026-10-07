#pragma once

#include "services/UserInfoService.hpp"

namespace services::components
{
///@brief Набор общих компонент, используемых сервисами
static const auto InfoComponents
    = [] () -> userver::components::ComponentList {
  return userver::components::ComponentList{}
      .Append<::services::info_role::UserInfoService> ();
  ;
};
}
