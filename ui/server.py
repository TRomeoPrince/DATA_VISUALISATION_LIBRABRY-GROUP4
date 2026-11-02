"""Small local web server for Group 4's C++ graph library.

Requirements: Python 3 (standard library only) and a compiled graph_ui_cli.
Open http://127.0.0.1:8765/ after starting this file.
"""
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path
import json
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
WEB_PAGE = ROOT / "ui" / "index.html"
CSV_FILE = ROOT / "data" / "input" / "ui_uploaded.csv"
OUTPUT = ROOT / "data" / "output"
MAX_BODY = 1024 * 1024  # 1 MB CSV upload limit


def executable():
    suffix = ".exe" if sys.platform == "win32" else ""
    options = [
        ROOT / "build" / ("graph_ui_cli" + suffix),
        ROOT / "build" / "Debug" / ("graph_ui_cli" + suffix),
        ROOT / "build" / "Release" / ("graph_ui_cli" + suffix),
    ]
    for candidate in options:
        if candidate.is_file():
            return str(candidate)
    raise RuntimeError("First build the project: cmake -S . -B build && cmake --build build")


class Handler(BaseHTTPRequestHandler):
    def send_data(self, status, data, content_type):
        body = data.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            self.send_data(200, WEB_PAGE.read_text(encoding="utf-8"),
                           "text/html; charset=utf-8")
        else:
            self.send_data(404, "Not found", "text/plain; charset=utf-8")

    def do_POST(self):
        if self.path != "/api/generate":
            self.send_data(404, "Not found", "text/plain; charset=utf-8")
            return
        try:
            count = int(self.headers.get("Content-Length", "0"))
            if not 0 < count <= MAX_BODY:
                raise ValueError("CSV request must be smaller than 1 MB.")
            data = json.loads(self.rfile.read(count))
            csv = data.get("csv")
            chart = data.get("chart")
            column = data.get("column")
            title = data.get("title", "")
            bins = data.get("bins", 6)
            if not isinstance(csv, str) or not csv.strip():
                raise ValueError("Please provide a CSV file.")
            if not isinstance(title, str) or len(title) > 90:
                raise ValueError("Graph title must be 90 characters or less.")
            if chart not in ("count", "histogram", "kde"):
                raise ValueError("Select a valid graph type.")
            if type(column) is not int or column < 0 or column > 500:
                raise ValueError("Select a valid CSV column.")
            if type(bins) is not int or bins < 1 or bins > 100:
                raise ValueError("Bins must be between 1 and 100.")
            CSV_FILE.parent.mkdir(parents=True, exist_ok=True)
            CSV_FILE.write_text(csv, encoding="utf-8")
            filename = {
                "count": "ui_countplot.svg",
                "histogram": "ui_histogram.svg",
                "kde": "ui_kdeplot.svg",
            }[chart]
            result = subprocess.run(
                [executable(), str(CSV_FILE), chart, str(column),
                 title or chart.capitalize() + " Plot", str(bins),
                 filename, "880"],
                cwd=str(ROOT / "build"),
                capture_output=True, text=True, timeout=20, check=False,
            )
            if result.returncode:
                raise ValueError(result.stderr.strip()[:350] or "Graph generation failed.")
            svg = (OUTPUT / filename).read_text(encoding="utf-8")
            self.send_data(200, json.dumps({"svg": svg, "filename": filename,
                                             "relativePath": "data/output/" + filename}),
                           "application/json; charset=utf-8")
        except (ValueError, RuntimeError, OSError, subprocess.TimeoutExpired,
                json.JSONDecodeError) as e:
            self.send_data(400, json.dumps({"error": str(e)}),
                           "application/json; charset=utf-8")


if __name__ == "__main__":
    print("Group 4 visualizer: http://127.0.0.1:8765/")
    print("Only this computer can reach the server. Press Ctrl+C to stop.")
    HTTPServer(("127.0.0.1", 8765), Handler).serve_forever()
