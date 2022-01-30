/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AMS_NODE_HANDLE_HPP
#define __AMS_NODE_HANDLE_HPP

#include <thallium.hpp>
#include <memory>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <ams/Client.hpp>
#include <ams/Exception.hpp>
#include <ams/AsyncRequest.hpp>
#include <conduit/conduit.hpp>

namespace ams {

namespace tl = thallium;

class Client;
class NodeHandleImpl;

/**
 * @brief A NodeHandle object is a handle for a remote node
 * on a server. It enables invoking the node's functionalities.
 */
class NodeHandle {

    friend class Client;

    public:

    /**
     * @brief Constructor. The resulting NodeHandle handle will be invalid.
     */
    NodeHandle();

    /**
     * @brief Copy-constructor.
     */
    NodeHandle(const NodeHandle&);

    /**
     * @brief Move-constructor.
     */
    NodeHandle(NodeHandle&&);

    /**
     * @brief Copy-assignment operator.
     */
    NodeHandle& operator=(const NodeHandle&);

    /**
     * @brief Move-assignment operator.
     */
    NodeHandle& operator=(NodeHandle&&);

    /**
     * @brief Destructor.
     */
    ~NodeHandle();

    /**
     * @brief Returns the client this database has been opened with.
     */
    Client client() const;


    /**
     * @brief Checks if the NodeHandle instance is valid.
     */
    operator bool() const;

    /**
     * @brief Sends an RPC to the node to make it print a hello message.
     */
    void sayHello() const;

    /**
     * @brief Requests the target node to compute the sum of two numbers.
     * If result is null, it will be ignored. If req is not null, this call
     * will be non-blocking and the caller is responsible for waiting on
     * the request.
     *
     * @param[in] x first integer
     * @param[in] y second integer
     * @param[out] result result
     * @param[out] req request for a non-blocking operation
     */
    void computeSum(int32_t x, int32_t y,
                    int32_t* result = nullptr,
                    AsyncRequest* req = nullptr) const;

    /**
     * @brief Requests the opening of an ascent operation with a
     * given set of options.
     *
     * @param[in] opts ascent options represented as a conduit::Node
     */
    void ams_open(conduit::Node opts) const;

    /**
     * @brief Requests the publishing of an mesh represented as a conduit Node
     *
     * @param[in] bp_mesh conduit::Node
     */
    void ams_publish(conduit::Node bp_mesh) const;

    /**
     * @brief Requests the execution of a set of actions represented as a conduit Node
     *
     * @param[in] actions conduit::Node
     */
    void ams_execute(conduit::Node actions) const;

    /**
     * @brief Requests the publishing of a mesh and the execution of a set of actions represented as a conduit Node
     * as an atomic operation.
     *
     * @param[in] bp_mesh conduit::Node
     * @param[in] actions conduit::Node
     */
    void ams_publish_and_execute(conduit::Node bp_mesh, conduit::Node actions) const;

    /**
     * @brief Requests the publishing of a mesh and the execution of a set of actions represented as a conduit Node
     * as an atomic operation.
     *
     * @param[in] open_opts conduit::Node
     * @param[in] bp_mesh conduit::Node
     * @param[in] actions conduit::Node
     * @param[in] ts      timestamp
     * @param[in/out] actions async request
     */
    void ams_open_publish_execute(conduit::Node open_opts, 
		    thallium::bulk bp_mesh, 
		    size_t mesh_size,
		    conduit::Node actions,
		    unsigned int ts,
		    AsyncRequest* req = nullptr) const;

    /**
     * @brief Requests the closing of ascent operation
     *
     * @param[in] bp_mesh conduit::Node
     */
    void ams_close() const;

    private:

    /**
     * @brief Constructor is private. Use a Client object
     * to create a NodeHandle instance.
     *
     * @param impl Pointer to implementation.
     */
    NodeHandle(const std::shared_ptr<NodeHandleImpl>& impl);

    std::shared_ptr<NodeHandleImpl> self;
};

}

#endif
