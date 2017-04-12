# opx-common-utils
This repository contains a utilities library used by the OPX project for thread queues, mutex support, and other items. It is recommended that all utility code (used by more then one OPX component) be placed in this repository. For example, the event service used by the CPS. The API that starts, stops, and the infrastructure for events are in the _event_ sources, while the user code is in the `opx-cps` repository.

## Packages
- libopx-common1\_*version*\_*arch*.deb — Utility libraries  
- libopx-common-dev\_*version*\_*arch*.deb — Exported header files

See [Architecture](https://github.com/open-switch/opx-docs/wiki/Architecture) for more information on the common utilities module.

(c) 2017 Dell
