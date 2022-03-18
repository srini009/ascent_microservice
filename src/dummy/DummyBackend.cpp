/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "DummyBackend.hpp"
#include <iostream>
#include <ascent/ascent.hpp>
#include <mpi.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

#define WARMUP_PERIOD 0
#define EAGER 0
#define LAZY 1
#define LAZYISH 2

AMS_REGISTER_BACKEND(dummy, DummyNode);


/*static ascent::Ascent get_ascent_lib() {
	static ascent::Ascent a;
	return a;
}*/

void DummyNode::sayHello() {
    std::cout << "Hello World" << std::endl;
}

ams::RequestResult<bool> DummyNode::ams_open(std::string opts) {
    conduit::Node n;
    n.parse(opts,"conduit_json");
    /* Regardless of whether or not the client uses MPI, me the server needs to use MPI
     * because I am launched as a separate MPI program */
    n["mpi_comm"] = 0;
    n["messages"] = "verbose";

    ascent_lib.open(n);

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<bool> DummyNode::ams_close() {

    ascent_lib.close();

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<bool> DummyNode::ams_publish(std::string bp_mesh) {
    conduit::Node n;
    n.parse(bp_mesh,"conduit_json");

    ascent_lib.publish(n);

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<bool> DummyNode::ams_execute(std::string actions) {
    conduit::Node n;
    n.parse(actions,"conduit_json");

    ascent_lib.execute(n);

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

static double calculate_percent_memory_util()
{
    char str1[100], str2[100], dummy1[20], dummy2[20], dummy3[20], dummy4[20];
    char * buf1 = str1;
    char * buf2 = str2;
    uint64_t memtotal, memfree;
    FILE* fp = fopen("/proc/meminfo","r");
    size_t s1 = 100, s2 = 100;
    getline(&buf1, &s1, fp);
    getline(&buf2, &s2, fp);
    sscanf(str1, "%s %lu %s", dummy1, &memtotal, dummy2);
    sscanf(str2, "%s %lu %s", dummy3, &memfree, dummy4);
    fclose(fp);
    return (1.0-((double)memfree/(double)memtotal))*100.0;
}

void DummyNode::ams_execute_one_request(MPI_Comm comm, ascent::Ascent& a_lib, int rank, int size) {

    int top_task_id = (pq.top()).m_task_id;
    int recv;
    MPI_Allreduce(&top_task_id, &recv, 1, MPI_INT, MPI_SUM, comm);
    if(recv != top_task_id*size) {
        if(rank == 0)
            std::cerr << "Skipping this request. Size of pq: " << pq.size() << std::endl;
    } else {
        if(rank == 0)
            std::cerr << "Request is valid. Proceeding with the Ascent computation. Num items in queue: " << pq.size() << std::endl;
    }
    /* Perform the ascent viz as a single, atomic operation within the context of the RPC */
    a_lib.open((pq.top()).m_open_opts);
    a_lib.publish((pq.top()).m_data);
    a_lib.execute((pq.top()).m_actions);
    a_lib.close();

    /* Pop the top element */
    pq.pop();
}

/* Go through priority queue and execute all the pending requests one by one */
/* If this is called AFTER all the clients have sent me their data, it is virtually
 * guaranteed to execute in the order of the client timestamp */
void DummyNode::ams_execute_pending_requests(size_t pool_size, MPI_Comm comm) {
    ascent::Ascent a_lib;
    int size;
    int rank;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    double start = MPI_Wtime();

    while(pq.size() != 0) {
        ams_execute_one_request(comm, a_lib, rank, size);
    }

    double end = MPI_Wtime();

    if(rank == 0)
        std::cerr << "Total server time for finishing pending requests: " << end-start << std::endl;

}

ams::RequestResult<bool> DummyNode::ams_open_publish_execute(std::string open_opts, std::string bp_mesh, size_t mesh_size, std::string actions, unsigned int ts, size_t pool_size, MPI_Comm comm) {
    conduit::Node n, n_mesh, n_opts;

    int size;
    int rank;
    int mode = std::stoi(std::string(getenv("AMS_SERVER_MODE")));

    ams::RequestResult<bool> result;
    result.value() = true;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    std::ofstream myfile;

    FILE *fp, *fp_pq, *fp_argoq, *fp_memq;

    std::string filename_cpp = std::to_string(rank) + "_server_state.txt";
    std::string pq_size_cpp = std::to_string(rank) + "_pq_size.txt";
    std::string argoq_size_cpp = std::to_string(rank) + "_argoq_size.txt";
    std::string memsize_cpp = std::to_string(rank) + "_memsize_size.txt";

    const char *filename = filename_cpp.c_str();
    const char *pq_size = pq_size_cpp.c_str();
    const char *argoq_size = argoq_size_cpp.c_str();
    const char *memq_size = memsize_cpp.c_str();

    fp = fopen(filename, "a");
    fp_pq = fopen(pq_size, "a");
    fp_argoq = fopen(argoq_size, "a");
    fp_memq = fopen(memq_size, "a");

    double start = MPI_Wtime();
    fprintf(fp, "1,%.10lf\n", MPI_Wtime());

    ascent::Ascent a_lib;
    n.parse(actions,"conduit_base64_json");
    n_mesh.parse(bp_mesh,"conduit_base64_json");
    n_opts.parse(open_opts,"conduit_base64_json");
    n_opts["mpi_comm"] = MPI_Comm_c2f(comm);

    fprintf(fp, "2,%.10lf\n", MPI_Wtime());

    /* Checking if all my peers are working on the same request. If not, skip! */
    int task_id = n_opts["task_id"].to_int();
    ConduitNodeData c(n_mesh, n_opts, n, ts, task_id);
    pq.push(c);

    fprintf(fp_pq, "%.10lf\n", (double)pq.size());
    fprintf(fp_argoq, "%.10lf\n", (double)pool_size);
    fprintf(fp_memq, "%.10lf\n", (double)calculate_percent_memory_util());
    fclose(fp);
    fclose(fp_pq);
    fclose(fp_argoq);
    fclose(fp_memq);

    if(mode == LAZY)
        return result;

    /* Check if there are too many pending requests to respond to. If so, I just return. If not, proceed with ascent computation */
    if(mode == LAZYISH) {
        int execute_ascent = 0;
        if(rank == 0 and pool_size < 5) { /* 5 is chosen as some arbitrary number */
	    execute_ascent = 1;
        }

        MPI_Bcast(&execute_ascent, 1, MPI_INT, 0, comm);
        if(execute_ascent == 0) {
            return result;
        }
    }

    /* Execute the code below if there is not much work in the Argobots pending queue */
    int top_task_id = (pq.top()).m_task_id;
   
    /* Make sure that are all on the same page regarding which client's request we are executing. */ 
    int recv;
    MPI_Allreduce(&top_task_id, &recv, 1, MPI_INT, MPI_SUM, comm);
    if(recv != top_task_id*size) {
        if(rank == 0)
            std::cerr << "Skipping this request. Size of pq: " << pq.size() << " and size of ABT pool: " << pool_size << std::endl;
        return result;
    } else {
	if(rank == 0) {
            std::cerr << "Request is valid. Size of pq: " << pq.size() << " and size of ABT pool: " << pool_size << std::endl;
	}
    }

    fprintf(fp, "3,%.10lf\n", MPI_Wtime());

    symbiomon_metric_update(this->m_server_state, (double)1.0);

    /* Perform the ascent viz as a single, atomic operation within the context of the RPC */
    a_lib.open((pq.top()).m_open_opts);
    a_lib.publish((pq.top()).m_data);
    a_lib.execute((pq.top()).m_actions);
    a_lib.close();

    symbiomon_metric_update(this->m_server_state, (double)0.0);

    fprintf(fp, "0,%.10lf\n", MPI_Wtime());

    /* Pop the top element */
    pq.pop();

    double end = MPI_Wtime();

    if(rank == 0)
        std::cerr << "Total server time for ascent call: " << end-start << std::endl;

    /*fclose(fp);
    fclose(fp_pq);
    fclose(fp_argoq);
    fclose(fp_memq);*/

    return result;
}

ams::RequestResult<bool> DummyNode::ams_publish_and_execute(std::string bp_mesh, std::string actions) {
    conduit::Node n, n_mesh;

    n.parse(actions,"conduit_json");
    n_mesh.parse(bp_mesh,"conduit_json");

    /* Publish and execute as a single atomic operation */
    ascent_lib.publish(n_mesh);
    ascent_lib.execute(n);

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<int32_t> DummyNode::computeSum(int32_t x, int32_t y) {
    ams::RequestResult<int32_t> result;
    result.value() = x + y;
    return result;
}

ams::RequestResult<bool> DummyNode::destroy() {
    ams::RequestResult<bool> result;
    result.value() = true;
    // or result.success() = true
    return result;
}

std::unique_ptr<ams::Backend> DummyNode::create(const thallium::engine& engine, const json& config) {
    (void)engine;

    return std::unique_ptr<ams::Backend>(new DummyNode(config, engine));
}

std::unique_ptr<ams::Backend> DummyNode::open(const thallium::engine& engine, const json& config) {
    (void)engine;
    return std::unique_ptr<ams::Backend>(new DummyNode(config));
}
