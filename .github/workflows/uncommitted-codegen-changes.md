<!-- This is posted as a PR comment when CI detects uncommitted codegen changes. -->
## Uncommitted codegen changes detected!

It looks like the generated header files in libraries/codegen-tooling don't match the CAN message definition file at codegen/can_messages.asciipb. If you've changed the CAN message definitions, please run `make codegen` locally and push the changes.

Alternatively, maybe you've edited can_messages.asciipb in the [codegen-tooling-msxiv][1] repo. This [has been deprecated][2]: please make your changes to codegen/can_messages.asciipb instead.

[1]: https://github.com/uw-midsun/codegen-tooling-msxiv
[2]: https://uwmidsun.atlassian.net/l/c/ghJhoqbT
