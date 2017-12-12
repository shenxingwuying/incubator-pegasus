/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Microsoft Corporation
 *
 * -=- Robust Distributed System Nucleus (rDSN) -=-
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * Description:
 *     Unit-test for rpc_address.
 *
 * Revision history:
 *     Nov., 2015, @qinzuoyan (Zuoyan Qin), first version
 *     xxxx-xx-xx, author, fix bug about xxx
 */

#include <dsn/tool-api/rpc_address.h>
#include <dsn/tool-api/group_address.h>
#include <dsn/tool-api/uri_address.h>
#include <gtest/gtest.h>

using namespace ::dsn;

static inline uint32_t host_ipv4(uint8_t sec1, uint8_t sec2, uint8_t sec3, uint8_t sec4)
{
    uint32_t ip = 0;
    ip |= (uint32_t)sec1 << 24;
    ip |= (uint32_t)sec2 << 16;
    ip |= (uint32_t)sec3 << 8;
    ip |= (uint32_t)sec4;
    return ip;
}

static inline bool operator==(dsn::rpc_address l, dsn::rpc_address r)
{
    if (l.type() != r.type())
        return false;

    switch (l.type()) {
    case HOST_TYPE_IPV4:
        return l.ip() == r.ip() && l.port() == r.port();
    case HOST_TYPE_URI:
        return strcmp(l.uri_address()->uri(), r.uri_address()->uri()) == 0;
    case HOST_TYPE_GROUP:
        return l.group_address() == r.group_address();
    default:
        return true;
    }
}

TEST(core, rpc_address_ipv4_from_host)
{
    // localhost --> 127.0.0.1
    ASSERT_EQ(host_ipv4(127, 0, 0, 1), rpc_address::ipv4_from_host("localhost"));

    // 127.0.0.1 --> 127.0.0.1
    ASSERT_EQ(host_ipv4(127, 0, 0, 1), rpc_address::ipv4_from_host("127.0.0.1"));
}

TEST(core, rpc_address_ipv4_from_network_interface)
{
#ifndef _WIN32
    ASSERT_EQ(host_ipv4(127, 0, 0, 1), rpc_address::ipv4_from_network_interface("lo"));
    ASSERT_EQ(host_ipv4(0, 0, 0, 0),
              rpc_address::ipv4_from_network_interface("not_exist_interface"));
#endif
}

TEST(core, rpc_address_to_string)
{
    {
        dsn::rpc_address addr;
        addr.assign_ipv4(host_ipv4(127, 0, 0, 1), 8080);
        ASSERT_EQ(std::string("127.0.0.1:8080"), addr.to_std_string());
    }

    {
        const char *uri = "http://localhost:8080/";
        dsn::rpc_address addr;
        addr.assign_uri(uri);
        ASSERT_EQ(std::string(uri), addr.to_std_string());
    }

    {
        const char *name = "test_group";
        dsn::rpc_address addr;
        addr.assign_group(name);
        ASSERT_EQ(std::string(name), addr.to_std_string());
    }

    {
        dsn::rpc_address addr;
        ASSERT_EQ(std::string("invalid address"), addr.to_std_string());
    }
}

TEST(core, dsn_address_build)
{
    {
        dsn::rpc_address addr;
        addr.assign_ipv4(host_ipv4(127, 0, 0, 1), 8080);
        ASSERT_EQ(HOST_TYPE_IPV4, addr.type());
        ASSERT_EQ(host_ipv4(127, 0, 0, 1), addr.ip());
        ASSERT_EQ(8080, addr.port());

        ASSERT_EQ(addr, dsn::rpc_address("localhost", 8080));
        ASSERT_EQ(addr, dsn::rpc_address("127.0.0.1", 8080));
        ASSERT_EQ(addr, dsn::rpc_address(host_ipv4(127, 0, 0, 1), 8080));
    }

    {
        const char *uri = "http://localhost:8080/";
        dsn::rpc_address addr;
        addr.assign_uri(uri);

        ASSERT_EQ(addr.type(), HOST_TYPE_URI);
        ASSERT_STREQ(uri, addr.uri_address()->uri());
        ASSERT_EQ(1, addr.uri_address()->get_count());
    }

    {
        const char *name = "test_group";
        dsn::rpc_address addr;
        addr.assign_group(name);

        ASSERT_EQ(HOST_TYPE_GROUP, addr.type());
        ASSERT_STREQ(name, addr.group_address()->name());
        ASSERT_EQ(1, addr.group_address()->get_count());
    }
}

TEST(core, rpc_group_address)
{
    rpc_address addr("127.0.0.1", 8080);
    rpc_address invalid_addr;
    rpc_address addr2("127.0.0.1", 8081);

    rpc_address t;
    t.assign_group("test_group");
    ASSERT_EQ(HOST_TYPE_GROUP, t.type());
    rpc_group_address *g = t.group_address();
    ASSERT_EQ(std::string("test_group"), g->name());
    ASSERT_EQ(1, g->get_count());

    // { }
    ASSERT_FALSE(g->remove(addr));
    ASSERT_FALSE(g->contains(addr));
    ASSERT_EQ(0u, g->members().size());
    ASSERT_EQ(invalid_addr, g->random_member());
    ASSERT_EQ(invalid_addr, g->next(addr));
    ASSERT_EQ(invalid_addr, g->leader());
    ASSERT_EQ(invalid_addr, g->possible_leader());

    // { addr }
    ASSERT_TRUE(g->add(addr));
    ASSERT_FALSE(g->add(addr));
    ASSERT_TRUE(g->contains(addr));
    ASSERT_EQ(1u, g->members().size());
    ASSERT_EQ(addr, g->members().at(0));
    ASSERT_EQ(addr, g->random_member());
    ASSERT_EQ(addr, g->next(addr));
    ASSERT_EQ(addr, g->next(invalid_addr));
    ASSERT_EQ(addr, g->next(addr2));
    ASSERT_EQ(invalid_addr, g->leader());
    ASSERT_EQ(addr, g->possible_leader());

    // { addr* }
    g->set_leader(addr);
    ASSERT_TRUE(g->contains(addr));
    ASSERT_EQ(1u, g->members().size());
    ASSERT_EQ(addr, g->members().at(0));
    ASSERT_EQ(addr, g->leader());
    ASSERT_EQ(addr, g->possible_leader());

    // { addr, addr2* }
    g->set_leader(addr2);
    ASSERT_TRUE(g->contains(addr));
    ASSERT_TRUE(g->contains(addr2));
    ASSERT_EQ(2u, g->members().size());
    ASSERT_EQ(addr, g->members().at(0));
    ASSERT_EQ(addr2, g->members().at(1));
    ASSERT_EQ(addr2, g->leader());
    ASSERT_EQ(addr2, g->possible_leader());
    ASSERT_EQ(addr, g->next(addr2));
    ASSERT_EQ(addr2, g->next(addr));

    // { addr, addr2 }
    g->set_leader(invalid_addr);
    ASSERT_TRUE(g->contains(addr));
    ASSERT_TRUE(g->contains(addr2));
    ASSERT_EQ(2u, g->members().size());
    ASSERT_EQ(addr, g->members().at(0));
    ASSERT_EQ(addr2, g->members().at(1));
    ASSERT_EQ(invalid_addr, g->leader());

    // { addr*, addr2 }
    g->set_leader(addr);
    ASSERT_TRUE(g->contains(addr));
    ASSERT_TRUE(g->contains(addr2));
    ASSERT_EQ(2u, g->members().size());
    ASSERT_EQ(addr, g->members().at(0));
    ASSERT_EQ(addr2, g->members().at(1));
    ASSERT_EQ(addr, g->leader());

    // { uri_addr }
    ASSERT_TRUE(g->remove(addr));
    ASSERT_FALSE(g->contains(addr));
    ASSERT_TRUE(g->contains(addr2));
    ASSERT_EQ(1u, g->members().size());
    ASSERT_EQ(addr2, g->members().at(0));
    ASSERT_EQ(invalid_addr, g->leader());

    // { }
    ASSERT_TRUE(g->remove(addr2));
    ASSERT_FALSE(g->contains(addr2));
    ASSERT_EQ(0u, g->members().size());
    ASSERT_EQ(invalid_addr, g->leader());
}
