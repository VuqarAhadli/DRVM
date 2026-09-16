import subprocess
from pathlib import Path
import re

ANSI_ESCAPE = re.compile(r"\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])")

class_files = {
    "GloftDIRU.class",
    "a.class",
    "b.class",
    "c.class",
    "d.class",
    "e.class",
    "f.class",
    "g.class",
    "h.class",
    "i.class",
}

extracted_dir = Path(__file__).parent / "extracted"
binary_dir = Path(__file__).parent / "build"

utf8_output_file = Path(__file__).parent / "inspection" / "utf8_output.txt"
methods_output_file = Path(__file__).parent / "inspection" / "methods_output.txt"


def run_drvm(drvm_path: Path, class_path: Path) -> str:
    result = subprocess.run(
        [str(drvm_path), str(class_path)],
        capture_output=True,
    )

    stdout = result.stdout.decode("utf-8", errors="replace")
    stderr = result.stderr.decode("utf-8", errors="replace")

    if result.returncode != 0:
        print(f"  drvm exited with code {result.returncode}")

        if stderr:
            print(f"  stderr: {stderr.strip()}")

    return stdout


def gather_utf8() -> None:
    drvm_path = binary_dir / "drvm"

    for class_file in class_files:
        class_path = extracted_dir / class_file

        if not class_path.exists():
            print(f"Class file {class_file} does not exist in {extracted_dir}.")
            continue

        print(f"Processing {class_file}...")

        stdout = run_drvm(drvm_path, class_path)

        for line in stdout.splitlines():
            if "Utf8" in line:
                line = ANSI_ESCAPE.sub("", line)
                print(f"  {line.strip()}")

                with open(utf8_output_file, "a", encoding="utf-8") as f:
                    f.write(f"  {line.strip()}\n")


def gather_methods() -> None:
    drvm_path = binary_dir / "drvm"

    for class_file in class_files:
        class_path = extracted_dir / class_file

        if not class_path.exists():
            print(f"Class file {class_file} does not exist in {extracted_dir}.")
            continue

        print(f"Processing {class_file}...")

        stdout = run_drvm(drvm_path, class_path)

        in_methods_section = False

        for line in stdout.splitlines():
            if line.strip().startswith("Methods:"):
                in_methods_section = True
                continue

            if in_methods_section:
                line = ANSI_ESCAPE.sub("", line)
                if line.strip().startswith("#") and " : " in line:
                    name = line.strip().split(" ")[1]

                    print(f"  method: {name}")

                    with open(methods_output_file, "a", encoding="utf-8") as f:
                        f.write(f"  method: {name}\n")

                elif (
                    line.strip() == ""
                    or line.strip().startswith("Class Attributes")
                ):
                    in_methods_section = False


if __name__ == "__main__":
    gather_utf8()
    gather_methods()