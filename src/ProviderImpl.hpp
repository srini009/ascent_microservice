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
#include <spdlog/spdlog.h>

#include <tuple>

#define FIND_NODE(__var__) \
        std::shared_ptr<Backend> __var__;\
        do {\
            std::lock_guard<tl::mutex> lock(m_backends_mtx);\
            auto it = m_backends.find(node_id);\
            if(it == m_backends.end()) {\
                result.success() = false;\
                result.error() = "Node with UUID "s + node_id.to_string() + " not found";\
                req.respond(result);\
                spdlog::error("[provider:{}] Node {} not found", id(), node_id.to_string());\
                return;\
            }\
            __var__ = it->second;\
        }while(0)

namespace ams {

using namespace std::string_literals;
namespace tl = thallium;

class ProviderImpl : public tl::provider<ProviderImpl> {

    auto id() const { return get_provider_id(); } // for convenience

    using json = nlohmann::json;

    public:

    std::string          m_token;
    tl::pool             m_pool;
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
    // Backends
    std::unordered_map<UUID, std::shared_ptr<Backend>> m_backends;
    tl::mutex m_backends_mtx;

    ProviderImpl(const tl::engine& engine, uint16_t provider_id, const tl::pool& pool)
    : tl::provider<ProviderImpl>(engine, provider_id)
    , m_pool(pool)
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
    {
        spdlog::trace("[provider:{0}] Registered provider with id {0}", id());
    }

    ~ProviderImpl() {
        spdlog::trace("[provider:{}] Deregistering provider", id());
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
        m_ams_publish.deregister();
        spdlog::trace("[provider:{}]    => done!", id());
    }

    void createNode(const tl::request& req,
                        const std::string& token,
                        const std::string& node_type,
                        const std::string& node_config) {

        spdlog::trace("[provider:{}] Received createNode request", id());
        spdlog::trace("[provider:{}]    => type = {}", id(), node_type);
        spdlog::trace("[provider:{}]    => config = {}", id(), node_config);

        auto node_id = UUID::generate();
        RequestResult<UUID> result;

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            spdlog::error("[provider:{}] Invalid security token {}", id(), token);
            return;
        }

        json json_config;
        try {
            json_config = json::parse(node_config);
        } catch(json::parse_error& e) {
            result.error() = e.what();
            result.success() = false;
            spdlog::error("[provider:{}] Could not parse node configuration for node {}",
                    id(), node_id.to_string());
            req.respond(result);
            return;
        }

        std::unique_ptr<Backend> backend;
        try {
            backend = NodeFactory::createNode(node_type, get_engine(), json_config);
        } catch(const std::exception& ex) {
            result.success() = false;
            result.error() = ex.what();
            spdlog::error("[provider:{}] Error when creating node {} of type {}:",
                    id(), node_id.to_string(), node_type);
            spdlog::error("[provider:{}]    => {}", id(), result.error());
            req.respond(result);
            return;
        }

        if(not backend) {
            result.success() = false;
            result.error() = "Unknown node type "s + node_type;
            spdlog::error("[provider:{}] Unknown node type {} for node {}",
                    id(), node_type, node_id.to_string());
            req.respond(result);
            return;
        } else {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);
            m_backends[node_id] = std::move(backend);
            result.value() = node_id;
        }
        
        req.respond(result);
        spdlog::trace("[provider:{}] Successfully created node {} of type {}",
                id(), node_id.to_string(), node_type);
    }

    void openNode(const tl::request& req,
                      const std::string& token,
                      const std::string& node_type,
                      const std::string& node_config) {

        spdlog::trace("[provider:{}] Received openNode request", id());
        spdlog::trace("[provider:{}]    => type = {}", id(), node_type);
        spdlog::trace("[provider:{}]    => config = {}", id(), node_config);

        auto node_id = UUID::generate();
        RequestResult<UUID> result;

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            spdlog::error("[provider:{}] Invalid security token {}", id(), token);
            return;
        }

        json json_config;
        try {
            json_config = json::parse(node_config);
        } catch(json::parse_error& e) {
            result.error() = e.what();
            result.success() = false;
            spdlog::error("[provider:{}] Could not parse node configuration for node {}",
                    id(), node_id.to_string());
            req.respond(result);
            return;
        }

        std::unique_ptr<Backend> backend;
        try {
            backend = NodeFactory::openNode(node_type, get_engine(), json_config);
        } catch(const std::exception& ex) {
            result.success() = false;
            result.error() = ex.what();
            spdlog::error("[provider:{}] Error when opening node {} of type {}:",
                    id(), node_id.to_string(), node_type);
            spdlog::error("[provider:{}]    => {}", id(), result.error());
            req.respond(result);
            return;
        }

        if(not backend) {
            result.success() = false;
            result.error() = "Unknown node type "s + node_type;
            spdlog::error("[provider:{}] Unknown node type {} for node {}",
                    id(), node_type, node_id.to_string());
            req.respond(result);
            return;
        } else {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);
            m_backends[node_id] = std::move(backend);
            result.value() = node_id;
        }
        
        req.respond(result);
        spdlog::trace("[provider:{}] Successfully created node {} of type {}",
                id(), node_id.to_string(), node_type);
    }

    void closeNode(const tl::request& req,
                        const std::string& token,
                        const UUID& node_id) {
        spdlog::trace("[provider:{}] Received closeNode request for node {}",
                id(), node_id.to_string());

        RequestResult<bool> result;

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            spdlog::error("[provider:{}] Invalid security token {}", id(), token);
            return;
        }

        {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);

            if(m_backends.count(node_id) == 0) {
                result.success() = false;
                result.error() = "Node "s + node_id.to_string() + " not found";
                req.respond(result);
                spdlog::error("[provider:{}] Node {} not found", id(), node_id.to_string());
                return;
            }

            m_backends.erase(node_id);
        }
        req.respond(result);
        spdlog::trace("[provider:{}] Node {} successfully closed", id(), node_id.to_string());
    }
    
    void destroyNode(const tl::request& req,
                         const std::string& token,
                         const UUID& node_id) {
        RequestResult<bool> result;
        spdlog::trace("[provider:{}] Received destroyNode request for node {}", id(), node_id.to_string());

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            spdlog::error("[provider:{}] Invalid security token {}", id(), token);
            return;
        }

        {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);

            if(m_backends.count(node_id) == 0) {
                result.success() = false;
                result.error() = "Node "s + node_id.to_string() + " not found";
                req.respond(result);
                spdlog::error("[provider:{}] Node {} not found", id(), node_id.to_string());
                return;
            }

            result = m_backends[node_id]->destroy();
            m_backends.erase(node_id);
        }

        req.respond(result);
        spdlog::trace("[provider:{}] Node {} successfully destroyed", id(), node_id.to_string());
    }

    void checkNode(const tl::request& req,
                       const UUID& node_id) {
        spdlog::trace("[provider:{}] Received checkNode request for node {}", id(), node_id.to_string());
        RequestResult<bool> result;
        FIND_NODE(node);
        result.success() = true;
        req.respond(result);
        spdlog::trace("[provider:{}] Code successfully executed on node {}", id(), node_id.to_string());
    }

    void sayHello(const tl::request& req,
                  const UUID& node_id) {
        spdlog::trace("[provider:{}] Received sayHello request for node {}", id(), node_id.to_string());
        RequestResult<bool> result;
        FIND_NODE(node);
        node->sayHello();
        spdlog::trace("[provider:{}] Successfully executed sayHello on node {}", id(), node_id.to_string());
    }

    /* SR: Core Ascent APIs */
    void ams_open(const tl::request& req,
                  const UUID& node_id,
		  std::string opts) {
        spdlog::trace("[provider:{}] Received ams_open request for node {}", id(), node_id.to_string());
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_open(opts);
	req.respond(result);
        spdlog::trace("[provider:{}] Successfully executed ams_open on node {}", id(), node_id.to_string());
    }

    void ams_close(const tl::request& req,
                  const UUID& node_id) {
        spdlog::trace("[provider:{}] Received ams_close request for node {}", id(), node_id.to_string());
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_close();
	req.respond(result);
        spdlog::trace("[provider:{}] Successfully executed ams_close on node {}", id(), node_id.to_string());
    }

    void ams_execute(const tl::request& req,
                  const UUID& node_id,
		  std::string actions) {
        spdlog::trace("[provider:{}] Received ams_execute request for node {}", id(), node_id.to_string());
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_execute(actions);
	req.respond(result);
        spdlog::trace("[provider:{}] Successfully executed ams_execute on node {}", id(), node_id.to_string());
    }

    void ams_open_publish_execute(const tl::request& req,
                  const UUID& node_id,
		  std::string open_opts,
		  std::string bp_mesh,
		  std::string actions) {
        spdlog::trace("[provider:{}] Received ams_open_publish_execute request for node {}", id(), node_id.to_string());
        RequestResult<bool> result;
        FIND_NODE(node);
        node->ams_open_publish_execute(open_opts, bp_mesh, actions);
        spdlog::trace("[provider:{}] Successfully executed ams_publish_and_execute on node {}", id(), node_id.to_string());
    }

    void ams_publish_and_execute(const tl::request& req,
                  const UUID& node_id,
		  std::string bp_mesh,
		  std::string actions) {
        spdlog::trace("[provider:{}] Received ams_publish_and_execute request for node {}", id(), node_id.to_string());
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_publish_and_execute(bp_mesh, actions);
	req.respond(result);
        spdlog::trace("[provider:{}] Successfully executed ams_publish_and_execute on node {}", id(), node_id.to_string());
    }

    void ams_publish(const tl::request& req,
                  const UUID& node_id,
		  std::string bp_mesh) {
        spdlog::trace("[provider:{}] Received ams_publish request for node {}", id(), node_id.to_string());
        RequestResult<bool> result;
        FIND_NODE(node);
        result = node->ams_publish(bp_mesh);
	req.respond(result);
        spdlog::trace("[provider:{}] Successfully executed ams_publish on node {}", id(), node_id.to_string());
    }

    void computeSum(const tl::request& req,
                    const UUID& node_id,
                    int32_t x, int32_t y) {
        spdlog::trace("[provider:{}] Received sayHello request for node {}", id(), node_id.to_string());
        RequestResult<int32_t> result;
        FIND_NODE(node);
        result = node->computeSum(x, y);
        req.respond(result);
        spdlog::trace("[provider:{}] Successfully executed computeSum on node {}", id(), node_id.to_string());
    }

};

}

#endif
