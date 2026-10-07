#include "DataBaseAccessable.hpp"

#include "env/Database.hpp"

#include <userver/storages/postgres/postgres.hpp>

using namespace services::common;

DataBaseAccessable::DataBaseAccessable (
    const userver::components::ComponentContext &context)
    : p_db (context
                .FindComponent<userver::components::Postgres> (
                    env::database::name)
                .GetCluster ())
{
}
