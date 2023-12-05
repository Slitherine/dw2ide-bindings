# dw2ide_runtime

`dw2ide_runtime` is a bridge module written mainly in C++ and JavaScript, envisioned for the [DW2IDE electron.js project](). This singular module aims to provide direct access to the .NET environment exported by a separate C# project. The project is open source and is released under the MIT License.

## Description

The project serves as glue for the .NET API of the C# DW2IDE project
and the JavaScript runtime of the DW2IDE electron.js project.

The project is written in C++ with a little bit of JavaScript for wiring up,
and is envisioned to be used as a dependency in the DW2IDE electron.js project.

## Installation

This project isn't meant to be used as a standalone project, but rather as a dependency in the DW2IDE electron.js project. However, if you want to use this project as a standalone project, you can do so by cloning this repository and building the project using the following commands:

*TODO: Add build instructions based on what we come up with for GitHub CI*

## Usage

After installing the project, you can use it in your JavaScript code by importing it like so:

```js
import * as dw2ide_runtime from 'dw2ide_runtime';
```

With this, you should be all set.

The project is written as modular javascript, so you can import only the parts you need.

See the API documentation for more information.

## Contributing

Contributions are always welcome.
Before any contribution, please read our [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

This project is released under the [MIT License](LICENSE).

## Contact

**[Distant Worlds 2 Official Discord](https://discord.gg/cnHU38YB6d)**


**[Distant Worlds 2 Official Forums](https://www.matrixgames.com/forums/viewforum.php?f=10151)**


**Slitherine Software UK Ltd.**<br/>
The Hermitage, 45 Church Street<br/>
Epsom, Surrey<br/>
KT17 4PW<br/>
United Kingdom

[Slitherine Support (support@slitherine.co.uk)](mailto:support@slitherine.co.uk)<br/>
