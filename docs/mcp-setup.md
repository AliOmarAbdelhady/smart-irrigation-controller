# Optional Velxio MCP setup

The browser editor is already enough to reproduce the verified visual run.
Velxio also provides a local MCP server for clients that support MCP.

The server is part of the Velxio repository at
<https://github.com/davidmonterocrespo24/velxio>. The documented entry point
is `backend/mcp_server.py` for stdio clients; the repository also documents an
SSE entry point at `backend/mcp_sse_server.py --port 8002`.

A generic stdio client configuration is:

```json
{
  "mcpServers": {
    "velxio": {
      "command": "python3",
      "args": ["/ABSOLUTE/PATH/TO/velxio/backend/mcp_server.py"]
    }
  }
}
```

Install the repository's backend requirements first, then replace the
placeholder with the absolute checkout path. The MCP server can compile or
run a project, but the browser editor remains the easiest way to see the
interactive circuit.

This project does not claim that a Codex MCP connection is currently active;
the verification in [`verification.md`](verification.md) was performed through
the hosted Velxio browser editor and local Arduino CLI compilation.
