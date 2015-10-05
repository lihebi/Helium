# configuration

By default, helium will use `$HELIUM_HOME/helium.xml` as config file.
This can be changed by given the config file in command line: `helium --config <path/to/config>`.

You need to copy from `helium.sample.xml` to `helium.xml`.

There're some predefined config scripts:

* buildrate.xml
* change.xml
* equivalence.xml

They can be invoked by given the run mode of Helium, e.g. `helium --build-rate <folder>`.

Config file will be loaded every time run Helium.
Config is a singleton, which means the config will be available for the lifetime of an instance of Helium.
