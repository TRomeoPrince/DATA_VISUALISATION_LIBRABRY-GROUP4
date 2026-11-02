
"""Local browser server for the Group 4 C++ Data Visualization Library."""

from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path
import json
import subprocess
import sys

# Locate the project folder automatically.
ROOT = Path(__file__).resolve().parent.parent

WEB_PAGE = ROOT / "ui" / "index.html"
CSV_FILE = ROOT / "data" / "input" / "ui_uploaded.csv"
OUTPUT = ROOT / "data" / "output"

MAX_BODY = 1024 * 1024


# Find the C++ executable created by CMake.
def find_executable():
    suffix = ".exe" if sys.platform == "win32" else ""

    possible_paths = [
        ROOT / "build" / ("graph_ui_cli" + suffix),
        ROOT / "build" / "Debug" / ("graph_ui_cli" + suffix),
        ROOT / "build" / "Release" / ("graph_ui_cli" + suffix),
        ROOT / "build-msys2" / ("graph_ui_cli" + suffix),
    ]

    for path in possible_paths:
        if path.is_file():
            return str(path)

    raise RuntimeError(
        "C++ UI executable not found. "
        "Run: cmake -S . -B build && cmake --build build"
    )


class Handler(BaseHTTPRequestHandler):

    # Send a response back to the browser.
    def send_data(self, status, data, content_type):
        body = data.encode("utf-8")

        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()

        self.wfile.write(body)

    # Show the main UI page.
    def do_GET(self):

        if self.path in ("/", "/index.html"):

            if not WEB_PAGE.is_file():
                self.send_data(
                    404,
                    "UI file not found: ui/index.html",
                    "text/plain; charset=utf-8"
                )
                return

            html = WEB_PAGE.read_text(encoding="utf-8")

            self.send_data(
                200,
                html,
                "text/html; charset=utf-8"
            )

        else:
            self.send_data(
                404,
                "Page not found",
                "text/plain; charset=utf-8"
            )

    # Receive the graph settings from the browser.
    def do_POST(self):

        if self.path != "/api/generate":
            self.send_data(
                404,
                "Not found",
                "text/plain; charset=utf-8"
            )
            return

        try:
            length = int(self.headers.get("Content-Length", "0"))

            if length <= 0 or length > MAX_BODY:
                raise ValueError(
                    "The CSV request must be smaller than 1 MB."
                )

            request_data = json.loads(
                self.rfile.read(length)
            )

            csv = request_data.get("csv")
            chart = request_data.get("chart")
            column = request_data.get("column")
            title = request_data.get("title", "")
            bins = request_data.get("bins", 6)

            # Check the values before calling the C++ program.
            if not isinstance(csv, str) or not csv.strip():
                raise ValueError("Please select a valid CSV file.")

            if chart not in ("count", "histogram", "kde"):
                raise ValueError("Invalid graph type.")

            if type(column) is not int or not 0 <= column <= 500:
                raise ValueError("Invalid CSV column.")

            if not isinstance(title, str) or len(title) > 90:
                raise ValueError("Invalid graph title.")

            if type(bins) is not int or not 1 <= bins <= 100:
                raise ValueError("Bins must be between 1 and 100.")

            # Create the input and output folders.
            CSV_FILE.parent.mkdir(
                parents=True,
                exist_ok=True
            )

            OUTPUT.mkdir(
                parents=True,
                exist_ok=True
            )

            # Save the selected CSV for the C++ DataLoader.
            CSV_FILE.write_text(
                csv,
                encoding="utf-8"
            )

            # Decide the filename based on the graph selected.
            filenames = {
                "count": "ui_countplot.svg",
                "histogram": "ui_histogram.svg",
                "kde": "ui_kdeplot.svg"
            }

            filename = filenames[chart]

            # Run the actual C++ library.
            result = subprocess.run(
                [
                    find_executable(),
                    str(CSV_FILE),
                    chart,
                    str(column),
                    title or chart.capitalize() + " Plot",
                    str(bins),
                    filename,
                    "880"
                ],
                cwd=str(ROOT),
                capture_output=True,
                text=True,
                timeout=20,
                check=False
            )

            if result.returncode != 0:
                raise ValueError(
                    result.stderr.strip()
                    or "Graph generation failed."
                )

            # Read the SVG produced by ImageExporter.
            svg_file = OUTPUT / filename

            if not svg_file.is_file():
                raise RuntimeError(
                    "The C++ program did not create the SVG file."
                )

            svg = svg_file.read_text(encoding="utf-8")

            # Send the SVG to the browser for preview.
            response = {
                "svg": svg,
                "filename": filename,
                "relativePath": "data/output/" + filename
            }

            self.send_data(
                200,
                json.dumps(response),
                "application/json; charset=utf-8"
            )

        except (
            ValueError,
            RuntimeError,
            OSError,
            subprocess.TimeoutExpired,
            json.JSONDecodeError
        ) as error:

            self.send_data(
                400,
                json.dumps({"error": str(error)}),
                "application/json; charset=utf-8"
            )


# Start the local web server.
if __name__ == "__main__":

    print("Starting Group 4 Data Visualization Library...")

    try:
        # Port 0 asks Windows to choose an available port.
        server = HTTPServer(
            ("127.0.0.1", 0),
            Handler
        )

        port = server.server_port

        print()
        print("=" * 55)
        print("GROUP 4 - DATA VISUALIZATION LIBRARY")
        print("=" * 55)
        print()
        print(f"Open your browser at: http://127.0.0.1:{port}/")
        print()
        print("Graphs will be saved inside: data/output/")
        print("Press CTRL+C to stop the server.")
        print("=" * 55)

        try:
            server.serve_forever()

        except KeyboardInterrupt:
            print("\nStopping the server...")

        finally:
            server.server_close()

    except OSError as error:
        print()
        print("Could not start the local server.")
        print("Windows error:", error)
        print("Check firewall or security restrictions.")
        sys.exit(1)
