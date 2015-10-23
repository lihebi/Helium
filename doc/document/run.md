# Run helium

Run `helium` to show help information.

`helium <folder>` to analyze the <folder>

## Common Workflow

```sh
export HELIUM_HOME=/path/to/Helium
make systype.tags # create system type tag file. Need only once on a new platform
cp helium.sample.xml helium.xml # and then change accordingly(especially interact option)
```

```sh
helium --pre <folder>
helium <folder>
```
