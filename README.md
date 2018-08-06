# Superboids

### Building
- Make: `make` or `make ada`
- Manual: `g++ src/*.cpp -o superboids`

### Running
`./superboids -sample` will generate the parameters structure.
The output might be redirected to a file.

`./superboids -param <file_with_parameters> -p` can be used to double check the parameters.

`./superboids -param <file_with_parameters> [OUTPUT_OPTIONS]` can be used to run a simulation,
where `[OUTPUT_OPTIONS]` can be any (one or more) in (and not limited to):
- `-binprint`: generate a binary output (extraction tool not included);
- `-plainprint`: generate a text output with positions, velocities and other informations;
- `-phi`: generate a text output with velocity allignment;
- `-shape`: generate a text output with shape information.

`./superboids -h` will guide you while this `README` is not fully documented.

### License
MIT

### Contact
- Main developer: cassio@kirch7.com
- Supervisor: leon@if.ufrgs.br
