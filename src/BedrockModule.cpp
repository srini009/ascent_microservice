/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "alpha/Client.hpp"
#include "alpha/Provider.hpp"
#include "alpha/ProviderHandle.hpp"
#include <bedrock/AbstractServiceFactory.hpp>

namespace tl = thallium;

class AlphaFactory : public bedrock::AbstractServiceFactory {

    public:

    AlphaFactory() {}

    void *registerProvider(const bedrock::FactoryArgs &args) override {
        auto provider = new alpha::Provider(args.mid, args.provider_id,
                args.config, tl::pool(args.pool));
        return static_cast<void *>(provider);
    }

    void deregisterProvider(void *p) override {
        auto provider = static_cast<alpha::Provider *>(p);
        delete provider;
    }

    std::string getProviderConfig(void *p) override {
        auto provider = static_cast<alpha::Provider *>(p);
        return provider->getConfig();
    }

    void *initClient(margo_instance_id mid) override {
        return static_cast<void *>(new alpha::Client(mid));
    }

    void finalizeClient(void *client) override {
        delete static_cast<alpha::Client *>(client);
    }

    void *createProviderHandle(void *c, hg_addr_t address,
            uint16_t provider_id) override {
        auto client = static_cast<alpha::Client *>(c);
        auto ph = new alpha::ProviderHandle(
                client->engine(),
                address,
                provider_id,
                false);
        return static_cast<void *>(ph);
    }

    void destroyProviderHandle(void *providerHandle) override {
        auto ph = static_cast<alpha::ProviderHandle *>(providerHandle);
        delete ph;
    }

    const std::vector<bedrock::Dependency> &getDependencies() override {
        static const std::vector<bedrock::Dependency> no_dependency;
        return no_dependency;
    }
};

BEDROCK_REGISTER_MODULE_FACTORY(alpha, AlphaFactory)
