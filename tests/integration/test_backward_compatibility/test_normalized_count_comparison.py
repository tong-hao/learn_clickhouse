import pytest

from helpers.cluster import ClickHouseCluster

cluster = ClickHouseCluster(__file__)
node1 = cluster.add_instance("node1", with_zookeeper=False, allow_analyzer=False)
node2 = cluster.add_instance(
    "node2",
    with_zookeeper=False,
    image="yandex/clickhouse-server",
    tag="21.6",
    stay_alive=True,
    with_installed_binary=True,
    allow_analyzer=False,
)


@pytest.fixture(scope="module")
def start_cluster():
    try:
        cluster.start()
        yield cluster

    finally:
        cluster.shutdown()


def test_select_aggregate_alias_column(start_cluster):
    node1.query(
        "create table tab (x UInt64, y String, z Nullable(Int64)) engine = Memory"
    )
    node2.query(
        "create table tab (x UInt64, y String, z Nullable(Int64)) engine = Memory"
    )
    node1.query("insert into tab values (1, 'a', null)")
    node2.query("insert into tab values (1, 'a', null)")

    node1.query(
        "select count(), count(1), count(x), count(y), count(z) from remote('node{1,2}', default, tab)"
    )
    node2.query(
        "select count(), count(1), count(x), count(y), count(z) from remote('node{1,2}', default, tab)"
    )

    node1.query("drop table tab")
    node2.query("drop table tab")
