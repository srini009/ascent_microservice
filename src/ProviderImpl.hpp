/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AMS_PROVIDER_IMPL_H
#define __AMS_PROVIDER_IMPL_H

#include "ams/Backend.hpp"
#include "ams/UUID.hpp"

#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/vector.hpp>

#include <nlohmann/json.hpp>

#include <ascent/ascent.hpp>

#include <tuple>
#include <mpi.h>

#define FIND_NODE(__var__) \
        std::shared_ptr<Backend> __var__;\
        do {\
            std::lock_guard<tl::mutex> lock(m_backends_mtx);\
            auto it = m_backends.find(node_id);\
            if(it == m_backends.end()) {\
                result.success() = false;\
                result.error() = "Node with UUID "s + node_id.to_string() + " not found";\
                req.respond(result);\
                return;\
            }\
            __var__ = it->second;\
        }while(0)

#define MAX_BULK_STRING_SIZE 1000000

namespace ams {

using namespace std::string_literals;
namespace tl = thallium;

class ProviderImpl : public tl::provider<ProviderImpl> {

    auto id() const { return get_provider_id(); } // for convenience

    using json = nlohmann::json;

    public:

    std::string          m_token;
    tl::pool             m_pool;
    MPI_Comm             m_comm;
    // Admin RPC
    tl::remote_procedure m_create_node;
    tl::remote_procedure m_open_node;
    tl::remote_procedure m_close_node;
    tl::remote_procedure m_destroy_node;
    // Client RPC
    tl::remote_procedure m_check_node;
    tl::remote_procedure m_say_hello;
    tl::remote_procedure m_compute_sum;
    /* SR: Core Ascent APIs */
    tl::remote_procedure m_ams_open;
    tl::remote_procedure m_ams_close;
    tl::remote_procedure m_ams_publish;
    tl::remote_procedure m_ams_execute;
    tl::remote_procedure m_ams_publish_and_execute;
    tl::remote_procedure m_ams_open_publish_execute;
    tl::remote_procedure m_ams_execute_pending_requests;
    // Backends
    std::unordered_map<UUID, std::shared_ptr<Backend>> m_backends;
    tl::mutex m_backends_mtx;

    ProviderImpl(const tl::engine& engine, uint16_t provider_id, MPI_Comm comm, const tl::pool& pool)
    : tl::provider<ProviderImpl>(engine, provider_id)
    , m_pool(pool), m_comm(comm)
    , m_create_node(define("ams_create_node", &ProviderImpl::createNode, pool))
    , m_open_node(define("ams_open_node", &ProviderImpl::openNode, pool))
    , m_close_node(define("ams_close_node", &ProviderImpl::closeNode, pool))
    , m_destroy_node(define("ams_destroy_node", &ProviderImpl::destroyNode, pool))
    , m_check_node(define("ams_check_node", &ProviderImpl::checkNode, pool))
    , m_say_hello(define("ams_say_hello", &ProviderImpl::sayHello, pool))
    , m_compute_sum(define("ams_compute_sum",  &ProviderImpl::computeSum, pool))
    , m_ams_open(define("ams_open",  &ProviderImpl::ams_open, pool))
    , m_ams_close(define("ams_close",  &ProviderImpl::ams_close, pool))
    , m_ams_publish(define("ams_publish",  &ProviderImpl::ams_publish, pool))
    , m_ams_execute(define("ams_execute",  &ProviderImpl::ams_execute, pool))
    , m_ams_publish_and_execute(define("ams_publish_and_execute",  &ProviderImpl::ams_publish_and_execute, pool))
    , m_ams_open_publish_execute(define("ams_open_publish_execute",  &ProviderImpl::ams_open_publish_execute, pool))
    , m_ams_execute_pending_requests(define("ams_execute_pending_requests",  &ProviderImpl::ams_execute_pending_requests, pool))
    {
    }

    ~ProviderImpl() {
        m_create_node.deregister();
        m_open_node.deregister();
        m_close_node.deregister();
        m_destroy_node.deregister();
        m_check_node.deregister();
        m_say_hello.deregister();
        m_compute_sum.deregister();
        m_ams_open.deregister();
        m_ams_close.deregister();
        m_ams_execute.deregister();
        m_ams_publish_and_execute.deregister();
        m_ams_open_publish_execute.deregister();
        m_ams_execute_pending_requests.deregister();
        m_ams_publish.deregister();
    }

    void createNode(const tl::request& req,
                        const std::string& token,
                        const std::string& node_type,
                        const std::string& node_config) {


        auto node_id = UUID::generate();
        RequestResult<UUID> result;

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            return;
        }

        json json_config;
        try {
            json_config = json::parse(node_config);
        } catch(json::parse_error& e) {
            result.error() = e.what();
            result.success() = false;
            req.respond(result);
            return;
        }

        std::unique_ptr<Backend> backend;
        try {
            backend = NodeFactory::createNode(node_type, get_engine(), json_config);
        } catch(const std::exception& ex) {
            result.success() = false;
            result.error() = ex.what();
            req.respond(result);
            return;
        }

        if(not backend) {
            result.success() = false;
            result.error() = "Unknown node type "s + node_type;
            req.respond(result);
            return;
        } else {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);
            m_backends[node_id] = std::move(backend);
            result.value() = node_id;
        }
        
        req.respond(result);
    }

    void openNode(const tl::request& req,
                      const std::string& token,
                      const std::string& node_type,
                      const std::string& node_config) {


        auto node_id = UUID::generate();
        RequestResult<UUID> result;

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            return;
        }

        json json_config;
        try {
            json_config = json::parse(node_config);
        } catch(json::parse_error& e) {
            result.error() = e.what();
            result.success() = false;
            req.respond(result);
            return;
        }

        std::unique_ptr<Backend> backend;
        try {
            backend = NodeFactory::openNode(node_type, get_engine(), json_config);
        } catch(const std::exception& ex) {
            result.success() = false;
            result.error() = ex.what();
            req.respond(result);
            return;
        }

        if(not backend) {
            result.success() = false;
            result.error() = "Unknown node type "s + node_type;
            req.respond(result);
            return;
        } else {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);
            m_backends[node_id] = std::move(backend);
            result.value() = node_id;
        }
        
        req.respond(result);
    }

    void closeNode(const tl::request& req,
                        const std::string& token,
                        const UUID& node_id) {

        RequestResult<bool> result;

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            return;
        }

        {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);

            if(m_backends.count(node_id) == 0) {
                result.success() = false;
                result.error() = "Node "s + node_id.to_string() + " not found";
                req.respond(result);
                return;
            }

            m_backends.erase(node_id);
        }
        req.respond(result);
    }
    
    void destroyNode(const tl::request& req,
                         const std::string& token,
                         const UUID& node_id) {
        RequestResult<bool> result;

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            return;
        }

        {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);

            if(m_backends.count(node_id) == 0) {
                result.success() = false;
                result.error() = "Node "s + node_id.to_string() + " not found";
                req.respond(result);
                return;
            }

            result = m_backends[node_id]->destroy();
            m_backends.erase(node_id);
        }

        req.respond(result);
    }

    void checkNode(const tl::request& req,
                       const UUID& node_id) {
        RequestResult<bool> result;
        FIND_NODE(node);
        result.success() = true;
        req.respond(result);
    }

    void sayHello(const tl::request& req,
                  const UUID& node_id) {
        RequestResult<bool> result;
        FIND_NODE(node);
        node->sayHello();
    }

    /* SR: Core Ascent APIs */
    void ams_open(const tl::request& req,
                  const UUID& node_id,
		  std::string opts) {
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_open(opts);
	req.respond(result);
    }

    void ams_close(const tl::request& req,
                  const UUID& node_id) {
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_close();
	req.respond(result);
    }

    void ams_execute(const tl::request& req,
                  const UUID& node_id,
		  std::string actions) {
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_execute(actions);
	req.respond(result);
    }

    void ams_open_publish_execute(const tl::request& req,
                  const UUID& node_id,
		  std::string open_opts,
		  std::string bp_mesh,
		  size_t mesh_size,
		  std::string actions,
		  unsigned int ts) {
        RequestResult<bool> result;
        FIND_NODE(node);

	auto engine = get_engine();
	auto pool = engine.get_handler_pool();
	result.value() = true;
	req.respond(result);
	node->ams_open_publish_execute(open_opts, bp_mesh, mesh_size, actions, ts, pool.total_size(), m_comm);
    }

    void ams_execute_pending_requests(const tl::request& req,
                  const UUID& node_id) {
        RequestResult<bool> result;
        FIND_NODE(node);
	auto engine = get_engine();
	auto pool = engine.get_handler_pool();
	node->ams_execute_pending_requests(engine, pool.total_size(), m_comm);
    }

    void ams_publish_and_execute(const tl::request& req,
                  const UUID& node_id,
		  std::string bp_mesh,
		  std::string actions) {
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_publish_and_execute(bp_mesh, actions);
	req.respond(result);
    }

    void ams_publish(const tl::request& req,
                  const UUID& node_id,
		  std::string bp_mesh) {
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_publish(bp_mesh);
	req.respond(result);
    }

    void computeSum(const tl::request& req,
                    const UUID& node_id,
                    int32_t x, int32_t y) {
        RequestResult<int32_t> result;
        FIND_NODE(node);
        result = node->computeSum(x, y);
        req.respond(result);
    }

};

}

#endif
