#include <co/co.hpp>
#include <co/redis/client.hpp>

using namespace std::chrono_literals;

const std::string ip = "127.0.0.1";
const uint16_t port = 6379;
const size_t n_clients = 3;
constexpr size_t n_requests = 1000;

co::func<void> client()
{
    auto client = co::redis::client(ip, port);
    co_await client.wait_until_connected();

    size_t ok = 0;
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
        {
            nok++;
        }
        else
            ok++;
    }

    if (nok > 0)
        std::cout << nok << " requests faield\n";

    client.close();
    co_await client.join();
}

int main()
{
    const auto start = std::chrono::steady_clock::now();

    co::loop(
        []() -> co::func<void>
        {
            for (int i = 0; i < n_clients; i++)
            {
                co::thread(client()).detach();
            }
            co_return;
        });

    const auto end = std::chrono::steady_clock::now();
    int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "FINISH " << (n_requests * n_clients * 2) << " requests in " << ms << "ms\n";
}
