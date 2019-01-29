ULFM Open MPI

This README.md documents the features and options specific to the
**User Level Failure Mitigation (ULFM)** Open MPI implementation.
The upstream (i.e. non-resilient) Open MPI directions also apply to
this release, except when specified here, and can be found in its
README file.

[TOC]

Features
========
This implementation conforms to the User Level Failure Mitigation (ULFM)
MPI Standard draft proposal. The ULFM proposal is developed by the MPI
Forum's Fault Tolerance Working Group to support the continued operation of
MPI programs after crash (node failures) have impacted the execution. The key
principle is that no MPI call (point-to-point, collective, RMA, IO, ...) can
block indefinitely after a failure, but must either succeed or raise an MPI
error.

This implementation produces the three supplementary error codes and five
supplementary interfaces defined in the communicator section of the
[http://fault-tolerance.org/wp-content/uploads/2012/10/20170221-ft.pdf]
(ULFM chapter) standard draft document.

+ `MPIX_ERR_PROC_FAILED` when a process failure prevents the completion of
  an MPI operation.
+ `MPIX_ERR_PROC_FAILED_PENDING` when a potential sender matching a non-blocking
  wildcard source receive has failed.
+ `MPIX_ERR_REVOKED` when one of the ranks in the application has invoked the
  `MPI_Comm_revoke` operation on the communicator.
+ `MPIX_Comm_revoke(MPI_Comm comm)` Interrupts any communication pending on
  the communicator at all ranks.
+ `MPIX_Comm_shrink(MPI_Comm comm, MPI_Comm* newcomm)` creates a new
  communicator where dead processes in comm were removed.
+ `MPIX_Comm_agree(MPI_Comm comm, int *flag)` performs a consensus (i.e. fault
  tolerant allreduce operation) on flag (with the operation bitwise or).
+ `MPIX_Comm_failure_get_acked(MPI_Comm, MPI_Group*)` obtains the group of
  currently acknowledged failed processes.
+ `MPIX_Comm_failure_ack(MPI_Comm)` acknowledges that the application intends
  to ignore the effect of currently known failures on wildcard receive
  completions and agreement return values.

## Supported Systems
There are four main MPI network models available in Open MPI: "ob1", "cm",
"yalla", and "ucx". Only "ob1" is adapted to support fault tolerance.

"ob1" uses BTL ("Byte Transfer Layer") components for each supported
network. "ob1" supports a variety of networks that can be used in
combination with each other. Collective operations (blocking and
non-blocking) use an optimized implementation on top of "ob1".

- Loopback (send-to-self)
- TCP
- OpenFabrics: InfiniBand, iWARP, and RoCE
- uGNI (Cray Gemini, Aries)
- Shared memory Vader (FT supported w/CMA, XPmem, KNEM untested)
- Tuned, and non-blocking collective communications

A full list of supported, untested and disabled components is provided
later in this document.

## More Information
More information (tutorials, examples, build instructions for leading
top500 systems) is also available in the Fault Tolerance Research
Hub website:
  <https://fault-tolerance.org>

## Bibliographic References
If you are looking for, or want to cite a general reference for ULFM,
please use

_Wesley Bland, Aurelien Bouteiller, Thomas Herault, George Bosilca, Jack
J. Dongarra: Post-failure recovery of MPI communication capability: Design
and rationale. IJHPCA 27(3): 244-254 (2013)._

Available from: http://journals.sagepub.com/doi/10.1177/1094342013488238.
___________________________________________________________________________

Building ULFM Open MPI
======================
```bash
./configure --with-ft [...options...]
#    use --with-ft (default) to enable ULFM, --without-ft to disable it
make [-j N] all install
#    use an integer value of N for parallel builds
```
There are many available configure options (see `./configure --help`
for a full list); a summary of the more commonly used ones is included
in the upstream Open MPI README file. The following paragraph gives a
summary of ULFM Open MPI specific options behavior.

## Configure options
+ `--with-ft=TYPE`
  Specify the type of fault tolerance to enable.  Options: mpi (ULFM MPI
  draft standard).  Fault tolerance support is enabled by default
  (as if `--with-ft=mpi` were implicitly present on the configure line).
  You may specify `--without-ft` to compile an almost stock Open MPI.

+ `--with-platform=FILE`
  Load configure options for the build from FILE.  When
  `--with-ft=mpi` is set, the file `contrib/platform/ft_mpi_ulfm` is
  loaded by default. This file disables components that are known to
  not be able to sustain failures, or are insufficiently tested.
  You may edit this file and/or force back these options on the
  command line to enable these components.

+ `--enable-mca-no-build=LIST`
  Comma-separated list of _<type>-<component>_ pairs that will not be
  built. For example, `--enable-mca-no-build=btl-portals,oob-ud` will
  disable building the _portals BTL_ and the _ud OOB_ component. When
  `--with-ft=mpi` is set, this list is populated with the content of
  the aforementioned platform file. You may override the default list
  with this parameter.

+ `--with-pmi`
  `--with-slurm`
  Force the building of SLURM scheduler support.
  Slurm with fault tolerance is tested. **Do not use `srun`**, otherwise your
  application gets killed by the scheduler upon the first failure. Instead,
  **Use `mpirun` in an `salloc/sbatch`**.

+ `-with-sge`
  This is untested with fault tolerance.

+ `--with-tm=<directory>`
  Force the building of PBS/Torque scheduler support.
  PBS is tested with fault tolerance. **Use `mpirun` in a `qsub`
  allocation.**

+ `--disable-mpi-thread-multiple`
  Disable the MPI thread level MPI_THREAD_MULTIPLE (it is enabled by
  default).
  Multiple threads with fault tolerance is lightly tested.

+ `--disable-oshmem`
  Disable building the OpenSHMEM implementation (by default, it is
  enabled).
  ULFM Fault Tolerance does not apply to OpenSHMEM.
___________________________________________________________________________

## Modified, Untested and Disabled Components
Frameworks and components which are not listed in the following list are
unmodified and support fault tolerance. Listed frameworks may be **modified**
(and work after a failure), **untested** (and work before a failure, but may
malfunction after a failure), or **disabled** (they prevent the fault
tolerant components from operating properly).

- **pml** MPI point-to-point management layer
    - "ob1" modified to **handle errors**
    - "monitoring", "v" unmodified, **untested**
    - "bfo", "cm", "crcpw", "ucx", "yalla" **disabled**

- **btl** Point-to-point Byte Transfer Layer
    - "openib", "tcp", "vader(+cma,+xpmem)" modified to **handle errors** (removed
      unconditional abort on error, expect performance similar to upstream)
    - "uct", "portals4", "scif", "smcuda", "usnic", "vader(+knem)" unmodified,
      **untested** (may work properly, please report)

- **mtl** Matching transport layer Used for MPI point-to-point messages on
  some types of networks
    - All "mtl" components are **disabled**

- **coll** MPI collective algorithms
    - "tuned", "basic", "nbc" modified to **handle errors**
    - "cuda", "fca", "hcoll", "portals4" unmodified, **untested** (expect
      unspecified post-failure behavior)

- **osc** MPI one-sided communications
    - Unmodified, **untested** (expect unspecified post-failure behavior)

- **io** MPI I/O and dependent components
    - _fs_ File system functions for MPI I/O
    - _fbtl_ File byte transfer layer: abstraction for individual read/write
      operations for OMPIO
    - _fcoll_ Collective read and write operations for MPI I/O
    - _sharedfp_ Shared file pointer operations for MPI I/O
    - All components in these frameworks are unmodified, **untested**
      (expect clean post-failure abort)

- **vprotocol** Checkpoint/Restart components
    - "pml-v", "crcp" unmodified, **untested**

- **wait_sync** Multithreaded wait-synchronization object
    - modified to **handle errors** (added a global interrupt to trigger all
      wait_sync objects)
___________________________________________________________________________

Running ULFM Open MPI
=====================

## Building your application

As ULFM is still an extension to the MPI standard, you will need to
`#include <mpi-ext.h>` in C, or `use mpi_ext` in Fortran to access the
supplementary error codes and functions.

Compile your application as usual, using the provided `mpicc`, `mpif90`, or
`mpicxx` wrappers.

## Running your application

You can launch your application with fault tolerance by simply using the
provided `mpiexec`. Beware that your distribution may already provide a
version of MPI, make sure to set your `PATH` and `LD_LIBRARY_PATH` properly.
Note that fault tolerance is enabled by default in ULFM Open MPI; you can
disable all fault tolerance systems by launching your application with
`mpiexec --disable-recovery`.

## Running under a batch scheduler

ULFM can operate under a job/batch scheduler, and is tested routinely with
both PBS and Slurm. One difficulty comes from the fact that many job
schedulers will "cleanup" the application as soon as a process fails. In
order to avoid this problem, it is preferred that you use `mpiexec`
within an allocation (e.g. `salloc`, `sbatch`, `qsub`) rather than
a direct launch (e.g. `srun`).

## Run-time tuning knobs

ULFM comes with a variety of knobs for controlling how it runs. The default
parameters are sane and should result in very good performance in most
cases. You can change the default settings with `--mca mpi_ft_foo <value>`.

- `orte_enable_recovery <true|false> (default: true)` controls automatic
  cleanup of apps with failed processes within mpirun. The default
  differs from upstream Open MPI.
- `mpi_ft_enable <true|false> (default: true)` permits turning off fault
  tolerance without recompiling. Failure detection is disabled. Interfaces
  defined by the fault tolerance extensions are substituted with dummy
  non-fault tolerant implementations (e.g. `MPI_Allreduce` is substituted
  to `MPIX_Comm_agree`).
- `mpi_ft_verbose <int> (default: 0)` increases the output of the fault
  tolerance activities. A value of 1 will report detected failures.
- `mpi_ft_detector <true|false> (default: false)` controls the activation
  of the OMPI level failure detector. When this detector if turned
  off, all failure detection is delegated to ORTE, which may be
  slow and/or incomplete.
- `mpi_ft_detector_thread <true|false> (default: false)` controls the use
  of a thread to emit and receive failure detector's heartbeats. _Setting
  this value to "true" will also set `MPI_THREAD_MULTIPLE` support, which
  has a noticeable effect on latency (typically 1us increase)._ You may
  want to **enable this option if you experience false positive**
  processes incorrectly reported as failed.
- `mpi_ft_detector_period <float> (default: 1e-1 seconds)` heartbeat
  period. Recommended value is 1/3 of the timeout. _Values lower than
  100us may impart a noticeable effect on latency (typically a 3us
  increase)._
- `mpi_ft_detector_timeout <float> (default: 3e-1 seconds)` heartbeat
  timeout (i.e. failure detection speed). Recommended value is 3 times
  the heartbeat period.

## Known Limitations in ULFM-2.1:
- Infiniband support is provided through the OpenIB BTL, fault tolerant 
  operation over UCX is not yet supported.
- TOPO, FILE, RMA are not fault tolerant.
- There is a tradeoff between failure detection accuracy and performance.
  The current default is to favor performance. Users that experience
  accuracy issues may enable a more precise mode.
- The failure detector operates on MPI_COMM_WORLD exclusively. Processes
  connected from MPI_COMM_CONNECT/ACCEPT and MPI_COMM_SPAWN may
  occasionally not be detected when they fail.
- Return of OpenIB credits spent toward a failed process can take several
  seconds. Until the `btl_openib_ib_timeout` and `btl_openib_ib_retry_count`
  controlled timeout triggers, lack of send credits may cause a temporary
  stall under certain communication patterns. Impacted users can try to
  adjust the value of these mca parameters.
___________________________________________________________________________


Changelog
=========

## Release 2.1
This release is a bugfix and upstream parity upgrade. It improves stability,
performance and is based on the most current Open MPI master (November 2018).

- ULFM is now based upon Open MPI master branch (#37954b5f).
- ULFM tuning MCA parameters are exposed by `ompi_info`.
- Fortran 90 bindings have been updated
- Bugfixes:
    - Correct the behavior of process placement during an MPI_COMM_SPAWN when 
      some slots were occcupied by failed processes.
    - MPI_COMM_SPAWN accepts process placement directives in the Info object.
    - Fixed deadlocks in some NBC collective operations.
    - Crashes and deadlocks in MPI_FINALIZE have been resolved.
    - Any-source requests that returned with an error status of 
      MPIX_PROC_FAILED_PENDING can now correctly complete during 
      later MPI_WAIT/TEST.

## Release 2.0
Focus has been toward integration with current Open MPI master (November 2017),
performance, and stability.

- ULFM is now based upon Open MPI master branch (#689f1be9). It will be
  regularly updated until it will eventually be merged.
- Fault Tolerance is enabled by default and is controlled with MCA variables.
- Added support for multithreaded modes (MPI_THREAD_MULTIPLE, etc.)
- Added support for non-blocking collective operations (NBC).
- Added support for CMA shared memory transport (Vader).
- Added support for advanced failure detection at the MPI level.
  Implements the algorithm described in "Failure detection and
  propagation in HPC systems." <https://doi.org/10.1109/SC.2016.26>.
- Removed the need for special handling of CID allocation.
- Non-usable components are automatically removed from the build during configure
- RMA, FILES, and TOPO components are enabled by default, and usage in a fault
  tolerant execution warns that they may cause undefined behavior after a failure.
- Bugfixes:
    - Code cleanup and performance cleanup in non-FT builds; --without-ft at
      configure time gives an almost stock Open MPI.
    - Code cleanup and performance cleanup in FT builds with FT runtime disabled;
      --mca ft_enable_mpi false thoroughly disables FT runtime activities.
    - Some error cases would return ERR_PENDING instead of ERR_PROC_FAILED in
      collective operations.
    - Some test could set ERR_PENDING or ERR_PROC_FAILED instead of
      ERR_PROC_FAILED_PENDING for ANY_SOURCE receptions.
___________________________________________________________________________

## Release 1.1
Focus has been toward improving stability, feature coverage for intercomms,
and following the updated specification for MPI_ERR_PROC_FAILED_PENDING.

- Forked from Open MPI 1.5.5 devel branch
- Addition of the MPI_ERR_PROC_FAILED_PENDING error code, as per newer specification
  revision. Properly returned from point-to-point, non-blocking ANY_SOURCE operations.
- Alias MPI_ERR_PROC_FAILED, MPI_ERR_PROC_FAILED_PENDING and MPI_ERR_REVOKED to the
  corresponding standard blessed -extension- names MPIX_ERR_xxx.
- Support for Intercommunicators:
    - Support for the blocking version of the agreement, MPI_COMM_AGREE on Intercommunicators.
    - MPI_COMM_REVOKE tested on intercommunicators.
- Disabled completely (.ompi_ignore) many untested components.
- Changed the default ORTE failure notification propagation aggregation delay from 1s to 25ms.
- Added an OMPI internal failure propagator; failure propagation between SM domains is now
  immediate.
- Bugfixes:
    - SendRecv would not always report MPI_ERR_PROC_FAILED correctly.
    - SendRecv could incorrectly update the status with errors pertaining to the Send portion
      of the Sendrecv.
    - Revoked send operations are now always completed or remote cancelled and may not
      deadlock anymore.
    - Cancelled send operations to a dead peer will not trigger an assert when the BTL reports
      that same failure.
    - Repeat calls to operations returning MPI_ERR_PROC_FAILED will eventually return
      MPI_ERR_REVOKED when another process revokes the communicator.
___________________________________________________________________________

## Release 1.0
Focus has been toward improving performance, both before and after the occurence of failures.
The list of new features includes:

- Support for the non-blocking version of the agreement, MPI_COMM_IAGREE.
- Compliance with the latest ULFM specification draft. In particular, the
  MPI_COMM_(I)AGREE semantic has changed.
- New algorithm to perform agreements, with a truly logarithmic complexity in number of
  ranks, which translates into huge performance boosts in MPI_COMM_(I)AGREE and
  MPI_COMM_SHRINK.
- New algorithm to perform communicator revocation. MPI_COMM_REVOKE performs a reliable
  broadcast with a fixed maximum output degree, which scales logarithmically with the
  number of ranks.
- Improved support for our traditional network layer:
    - TCP: fully tested
    - SM: fully tested (with the exception of XPMEM, which remains unsupported)
- Added support for High Performance networks
    - Open IB: reasonably tested
    - uGNI: reasonably tested
- The tuned collective module is now enabled by default (reasonably tested), expect a
  huge performance boost compared to the former basic default setting<
    - Back-ported PBS/ALPS fixes from Open MPI
    - Back-ported OpenIB bug/performance fixes from Open MPI
    - Improve Context ID allocation algorithm to reduce overheads of Shrink
    - Miscellaneous bug fixes
___________________________________________________________________________

## Version Numbers and Binary Compatibility
Starting from ULFM Open MPI version 2.0, ULFM Open MPI is binary compatible
with the corresponding Open MPI master branch and compatible releases (see
the binary compatibility and version number section in the upstream Open MPI
README).That is, applications compiled with a compatible Open MPI can run
with the ULFM Open MPI `mpirun` and MPI libraries. Conversely, _as long as
the application does not employ one of the MPIX functions_, which are
exclusively defined in ULFM Open MPI, an application compiled with
ULFM Open MPI can be launched with a compatible Open MPI `mpirun` and run
with the non-fault tolerant MPI library.
___________________________________________________________________________

Contacting the Authors
======================
Found a bug?  Got a question?  Want to make a suggestion?  Want to
contribute to ULFM Open MPI?  Working on a cool use-case?
Please let us know!

The best way to report bugs, send comments, or ask questions is to
sign up on the user's mailing list:
  <ulfm+subscribe@googlegroups.com>

Because of spam, only subscribers are allowed to post to these lists
(ensure that you subscribe with and post from exactly the same e-mail
address -- joe@example.com is considered different than
joe@mycomputer.example.com!).  Visit these pages to subscribe to the
lists:
  <https://groups.google.com/forum/#!forum/ulfm>

When submitting questions and problems, be sure to include as much
extra information as possible.  This web page details all the
information that we request in order to provide assistance:
  <http://www.open-mpi.org/community/help/>

Thanks for your time.
___________________________________________________________________________

Copyright
=========

```
Copyright (c) 2012-2018 The University of Tennessee and The University
                        of Tennessee Research Foundation.  All rights
                        reserved.

$COPYRIGHT$

Additional copyrights may follow

$HEADER$
```