# co_redis

co_redis is a asynchronous redis client based on [co_lib](https://github.com/dmitryikh/co_lib)

co_redis benefits:
1. ~800k RPS in one thread. Where `redis-bench` shows about 1000k RPS.
2. co::future interface with cancellation support
3. trying to restore a connection inside a client

Current limitations:
1. set/get comamnds only, but easy to extend
2. see `co_lib` limitations

# Dependencies
1. co_lib 0.1
2. boost 1.75
3. conan package manager


# Examples

```cpp
#include <co/co.hpp>
#include <co/redis/client.hpp>

using namespace std::chrono_literals;

const std::string ip = "127.0.0.1";
const uint16_t port = 6379;
constexpr size_t n_requests = 1000;

co::func<void> client()
{
    auto client = co::redis::client(ip, port);
    co_await client.wait_until_connected();

    size_t nok = 0;
    std::vector<co::future<co::result<co::redis::reply>>> replies;
    replies.reserve(n_requests * 2);

    for (int i = 0; i < n_requests; i++)
    {
        const std::string key = std::to_string(i);
        auto f1 = co_await client.set(key, "hello " + std::to_string(i));
        auto f2 = co_await client.get(key);
        replies.push_back(std::move(f1));
        replies.push_back(std::move(f2));
    }
    std::cout << "do flush\n";
    co_await client.flush();

    const auto deadline = std::chrono::steady_clock::now() + 10s;
    std::cout << "wait replies for 10s\n";
    for (auto& f : replies)
    {
        auto r = co_await f.get(co::until::deadline(deadline));

        if (r.is_err())
            nok++;
    }

    if (nok > 0)
        std::cout << nok << " requests faield\n";

    client.close();
    co_await client.join();
}

int main()
{
    const auto start = std::chrono::steady_clock::now();

    co::loop(client());

    const auto end = std::chrono::steady_clock::now();
    int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "FINISH " << (n_requests * 2) << " requests in " << ms << "ms\n";
}

```
