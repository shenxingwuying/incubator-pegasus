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
 *     What is this file about?
 *
 * Revision history:
 *     xxxx-xx-xx, author, first version
 *     xxxx-xx-xx, author, fix bug about xxx
 */
#ifndef replication_OTHER_TYPES_H
#define replication_OTHER_TYPES_H

#include <dsn/utility/autoref_ptr.h>
#include <dsn/dist/replication/replication.types.h>
#include <sstream>
#include <dsn/utility/enum_helper.h>

namespace dsn {

ENUM_BEGIN2(app_status::type, app_status, app_status::AS_INVALID)
ENUM_REG(app_status::AS_AVAILABLE)
ENUM_REG(app_status::AS_CREATING)
ENUM_REG(app_status::AS_CREATE_FAILED)
ENUM_REG(app_status::AS_DROPPING)
ENUM_REG(app_status::AS_DROP_FAILED)
ENUM_REG(app_status::AS_DROPPED)
ENUM_REG(app_status::AS_RECALLING)
ENUM_END2(app_status::type, app_status)

ENUM_BEGIN2(replication::partition_status::type,
            partition_status,
            replication::partition_status::PS_INVALID)
ENUM_REG(replication::partition_status::PS_INACTIVE)
ENUM_REG(replication::partition_status::PS_ERROR)
ENUM_REG(replication::partition_status::PS_PRIMARY)
ENUM_REG(replication::partition_status::PS_SECONDARY)
ENUM_REG(replication::partition_status::PS_POTENTIAL_SECONDARY)
ENUM_END2(replication::partition_status::type, partition_status)

ENUM_BEGIN2(replication::read_semantic::type,
            read_semantic,
            replication::read_semantic::ReadInvalid)
ENUM_REG(replication::read_semantic::ReadLastUpdate)
ENUM_REG(replication::read_semantic::ReadOutdated)
ENUM_REG(replication::read_semantic::ReadSnapshot)
ENUM_END2(replication::read_semantic::type, read_semantic)

ENUM_BEGIN2(replication::learn_type::type, learn_type, replication::learn_type::LT_INVALID)
ENUM_REG(replication::learn_type::LT_CACHE)
ENUM_REG(replication::learn_type::LT_APP)
ENUM_REG(replication::learn_type::LT_LOG)
ENUM_END2(replication::learn_type::type, learn_type)

ENUM_BEGIN2(replication::learner_status::type,
            learner_status,
            replication::learner_status::LearningInvalid)
ENUM_REG(replication::learner_status::LearningWithoutPrepare)
ENUM_REG(replication::learner_status::LearningWithPrepareTransient)
ENUM_REG(replication::learner_status::LearningWithPrepare)
ENUM_REG(replication::learner_status::LearningSucceeded)
ENUM_REG(replication::learner_status::LearningFailed)
ENUM_END2(replication::learner_status::type, learner_status)

ENUM_BEGIN2(replication::config_type::type, config_type, replication::config_type::CT_INVALID)
ENUM_REG(replication::config_type::CT_ASSIGN_PRIMARY)
ENUM_REG(replication::config_type::CT_UPGRADE_TO_PRIMARY)
ENUM_REG(replication::config_type::CT_ADD_SECONDARY)
ENUM_REG(replication::config_type::CT_UPGRADE_TO_SECONDARY)
ENUM_REG(replication::config_type::CT_DOWNGRADE_TO_SECONDARY)
ENUM_REG(replication::config_type::CT_DOWNGRADE_TO_INACTIVE)
ENUM_REG(replication::config_type::CT_REMOVE)
ENUM_REG(replication::config_type::CT_ADD_SECONDARY_FOR_LB)
ENUM_REG(replication::config_type::CT_PRIMARY_FORCE_UPDATE_BALLOT)
ENUM_REG(replication::config_type::CT_DROP_PARTITION)
ENUM_END2(replication::config_type::type, config_type)

ENUM_BEGIN2(replication::node_status::type, node_status, replication::node_status::NS_INVALID)
ENUM_REG(replication::node_status::NS_ALIVE)
ENUM_REG(replication::node_status::NS_UNALIVE)
ENUM_END2(replication::node_status::type, node_status)

namespace replication {

typedef int32_t app_id;
typedef int64_t ballot;
typedef int64_t decree;

#define invalid_ballot ((::dsn::replication::ballot)-1LL)
#define invalid_decree ((::dsn::replication::decree)-1LL)
#define invalid_offset (-1LL)
#define invalid_signature 0

class replica;
typedef dsn::ref_ptr<replica> replica_ptr;

class replica_stub;
typedef dsn::ref_ptr<replica_stub> replica_stub_ptr;

class mutation;
typedef dsn::ref_ptr<mutation> mutation_ptr;

class mutation_log;
typedef dsn::ref_ptr<mutation_log> mutation_log_ptr;

inline bool is_primary(const partition_configuration &pc, const rpc_address &node)
{
    return !node.is_invalid() && pc.primary == node;
}
inline bool is_secondary(const partition_configuration &pc, const rpc_address &node)
{
    return !node.is_invalid() &&
           std::find(pc.secondaries.begin(), pc.secondaries.end(), node) != pc.secondaries.end();
}
inline bool is_member(const partition_configuration &pc, const rpc_address &node)
{
    return is_primary(pc, node) || is_secondary(pc, node);
}
inline bool is_partition_config_equal(const partition_configuration &pc1,
                                      const partition_configuration &pc2)
{
    // secondaries no need to be same order
    for (const rpc_address &addr : pc1.secondaries)
        if (!is_secondary(pc2, addr))
            return false;
    // last_drops is not considered into equality check
    return pc1.ballot == pc2.ballot && pc1.pid == pc2.pid &&
           pc1.max_replica_count == pc2.max_replica_count && pc1.primary == pc2.primary &&
           pc1.secondaries.size() == pc2.secondaries.size() &&
           pc1.last_committed_decree == pc2.last_committed_decree;
}

class replica_helper
{
public:
    static bool remove_node(::dsn::rpc_address node,
                            /*inout*/ std::vector<::dsn::rpc_address> &nodeList);
    static bool get_replica_config(const partition_configuration &partition_config,
                                   ::dsn::rpc_address node,
                                   /*out*/ replica_configuration &replica_config);
    static void load_meta_servers(/*out*/ std::vector<dsn::rpc_address> &servers,
                                  const char *section = "meta_server",
                                  const char *key = "server_list");
};
}
} // namespace

#endif
