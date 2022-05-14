SERVIZ: A Shared In situ Visualization Service
==============================
This is an experimental repo implementing a distributed Ascent visualization microservice.

Inline and in transit visualization have arisen as popular models of in situ visualization for high performance computing (HPC) applications. Inline visualization is invoked through a library call on the HPC application (simulation code), while in transit methods involve the invocation of the visualization module on in transit resources. Compared to inline methods, in transit methods have the flexibility to run at a lower level of concurrency than the simulation code, allowing them to offer better efficiency for the visualization operation. The state-of-the-art in transit schemes are limited to employing a dedicated in transit resource for every HPC application.
This results in significant idle time on the in transit resource and severely limits the cost savings that can be achieved over the inline model.
This research proposes that a single, in transit visualization service be shared amongst multiple HPC applications to make efficient use of the in transit resources by reducing the idle time. We realize this idea through SERVIZ, a shared in transit visualization service. SERVIZ achieves cost savings of up to 40% over inline (at scale) and up to 4x reduction in idle time compared to a dedicated in transit implementation.
In all, the results from this work identify that a shared in transit resource is an attractive approach for cost efficiency.
