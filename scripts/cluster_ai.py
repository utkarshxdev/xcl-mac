#!/usr/bin/env python3
"""
Cluster AI Swarm Client for xcl-mac development.
Interacts with:
  - Coder (32B): http://10.6.0.83:30000/v1 (GPUs 0, 1)
  - Reviewer (7B): http://10.6.0.83:30001/v1 (GPU 5)
"""

import sys
import json
import argparse
import urllib.request
import urllib.error

CLUSTER_HOST = "10.6.0.83"
PORT_CODER = 30000
PORT_REVIEWER = 30001

def check_status(port, name):
    url = f"http://{CLUSTER_HOST}:{port}/v1/models"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "xcl-client"})
        with urllib.request.urlopen(req, timeout=3) as resp:
            data = json.loads(resp.read().decode())
            model_id = data.get("data", [{}])[0].get("id", "Unknown")
            return True, f"ONLINE ({model_id})"
    except Exception as e:
        return False, f"OFFLINE ({e})"

def query_model(port, model_name, messages, max_tokens=2048, temperature=0.2):
    url = f"http://{CLUSTER_HOST}:{port}/v1/chat/completions"
    payload = {
        "model": model_name,
        "messages": messages,
        "max_tokens": max_tokens,
        "temperature": temperature,
    }
    body = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(
        url,
        data=body,
        headers={"Content-Type": "application/json", "User-Agent": "xcl-client"},
    )
    try:
        with urllib.request.urlopen(req, timeout=120) as resp:
            data = json.loads(resp.read().decode())
            return data["choices"][0]["message"]["content"]
    except urllib.error.HTTPError as e:
        return f"HTTP Error {e.code}: {e.read().decode()}"
    except Exception as e:
        return f"Connection Error: {e}"

def cmd_status():
    print(f"=== Cluster AI Status ({CLUSTER_HOST}) ===")
    ok_coder, msg_coder = check_status(PORT_CODER, "Coder (32B)")
    print(f"GPUs 0,1 [Port {PORT_CODER}] Coder (32B)    : {msg_coder}")
    ok_rev, msg_rev = check_status(PORT_REVIEWER, "Reviewer (7B)")
    print(f"GPU 5    [Port {PORT_REVIEWER}] Reviewer (7B) : {msg_rev}")

def cmd_ask(args):
    target_port = PORT_CODER if args.model == "coder" else PORT_REVIEWER
    model_name = "Qwen/Qwen2.5-Coder-32B-Instruct" if args.model == "coder" else "Qwen/Qwen2.5-Coder-7B-Instruct"
    
    ok, status = check_status(target_port, args.model)
    if not ok:
        print(f"Error: Model {args.model} on port {target_port} is not online yet: {status}")
        sys.exit(1)

    prompt = args.prompt
    if args.file:
        with open(args.file, "r") as f:
            prompt = f.read() + "\n\n" + prompt

    print(f"Querying {args.model} ({model_name})...")
    messages = [
        {"role": "system", "content": "You are an expert systems and graphics translation layer engineer specialized in Apple Silicon, Wine/D3DMetal, DirectX 12, and AMD GFX10/RDNA2 GPU architecture."},
        {"role": "user", "content": prompt}
    ]
    response = query_model(target_port, model_name, messages, max_tokens=args.max_tokens, temperature=args.temp)
    print("\n--- Response ---")
    print(response)

def main():
    parser = argparse.ArgumentParser(description="Query cluster AI models")
    subparsers = parser.add_subparsers(dest="cmd", help="Subcommand")

    subparsers.add_parser("status", help="Check cluster AI status")

    ask_p = subparsers.add_parser("ask", help="Query a model")
    ask_p.add_argument("prompt", help="Prompt to send")
    ask_p.add_argument("--model", choices=["coder", "reviewer"], default="reviewer", help="Target model (default: reviewer)")
    ask_p.add_argument("--file", help="Optional file context to include")
    ask_p.add_argument("--max-tokens", type=int, default=2048)
    ask_p.add_argument("--temp", type=float, default=0.2)

    args = parser.parse_args()
    if args.cmd == "status" or not args.cmd:
        cmd_status()
    elif args.cmd == "ask":
        cmd_ask(args)

if __name__ == "__main__":
    main()
