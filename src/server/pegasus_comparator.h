/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#pragma once

#include <rocksdb/slice.h>
#include <rocksdb/comparator.h>
#include <rocksdb/coding.h>

namespace pegasus {
namespace server {

class PegasusComparator : public rocksdb::Comparator {
public:
    PegasusComparator() {}
    virtual const char* Name() const {
        return "PegasusComparator";
    }
    virtual int Compare(const Slice& a, const Slice& b) const {
        rocksdb::Slice a_slice(a);
        rocksdb::Slice b_slice(b);
        uint16_t a_length;
        rocksdb::GetFixed16(a_slice, &a_length);
        uint16_t b_length;
        rocksdb::GetFixed16(b_slice, &b_length);

        rocksdb::Slice a_hash_key(a_slice.data(), a_length);
        rocksdb::Slice b_hash_key(b_slice.data(), b_length);
        int ret = a_hash_key.compare(b_hash_key);
        if (ret != 0) {
            return ret;
        }
        a_slice.remove_prefix(a_length);
        b_slice.remove_prefix(b_length);

        return a_slice.compare(b_slice);
    }
    virtual void FindShortestSeparator(std::string* start,
                                       const Slice& limit) const {
        (void) start;
        (void) limit;
        // TODO
    }
    virtual void FindShortSuccessor(std::string* key) const {
        (void) key;
        // TODO
    }

};

}
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
