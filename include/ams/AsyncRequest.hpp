/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AMS_ASYNC_REQUEST_HPP
#define __AMS_ASYNC_REQUEST_HPP

#include <memory>
#include <string>

namespace ams {

class AsyncRequestImpl;
class NodeHandle;

/**
 * @brief AsyncRequest objects are used to keep track of
 * on-going asynchronous operations.
 */
class AsyncRequest {

    friend NodeHandle;

    public:

    /**
     * @brief Default constructor. Will create a non-valid AsyncRequest.
     */
    AsyncRequest();

    /**
     * @brief Copy constructor.
     */
    AsyncRequest(const AsyncRequest& other);

    /**
     * @brief Move constructor.
     */
    AsyncRequest(AsyncRequest&& other);

    /**
     * @brief Copy-assignment operator.
     */
    AsyncRequest& operator=(const AsyncRequest& other);

    /**
     * @brief Move-assignment operator.
     */
    AsyncRequest& operator=(AsyncRequest&& other);

    /**
     * @brief Destructor.
     */
    ~AsyncRequest();

    /**
     * @brief Wait for the request to complete.
     */
    void wait() const;

    /**
     * @brief Test if the request has completed, without blocking.
     */
    bool completed() const;

    /**
     * @brief Checks if the Collection object is valid.
     */
    operator bool() const;

    private:

    std::shared_ptr<AsyncRequestImpl> self;

    AsyncRequest(const std::shared_ptr<AsyncRequestImpl>& impl);

};

}

#endif
