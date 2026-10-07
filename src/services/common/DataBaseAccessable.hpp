#pragma once

#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/component.hpp>

namespace services::common
{

class DataBaseAccessable
{
public:
  explicit DataBaseAccessable (
      const userver::components::ComponentContext &context);
  DataBaseAccessable (const DataBaseAccessable &) = delete;
  DataBaseAccessable (DataBaseAccessable &&) = delete;

  virtual ~DataBaseAccessable () = default;

protected:
  const userver::storages::postgres::ClusterPtr
  db () const
  {
    return p_db;
  }

private:
  userver::storages::postgres::ClusterPtr p_db = nullptr;
};

}
