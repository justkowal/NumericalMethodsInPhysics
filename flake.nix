{
  description = "Numerical Methods in Physics (NMP) lab development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
      };

      # Pin to Python 3.12 for stable pre-compiled NumPy/SciPy/Matplotlib C-extensions
      pythonEnv = pkgs.python312.withPackages (ps:
        with ps; [
          numpy
          scipy
          matplotlib
        ]);

      # Include and library discovery for compilers, LSPs, and VS Code
      gccVersion = pkgs.gcc.version;
      gccInclude = "${pkgs.gcc.cc}/include/c++/${gccVersion}";
      gccArchInclude = "${pkgs.gcc.cc}/include/c++/${gccVersion}/${pkgs.stdenv.targetPlatform.config}";
      glibcInclude = "${pkgs.glibc.dev}/include";
      openblasInclude = "${pkgs.openblas.dev}/include";
      fftwInclude = "${pkgs.fftw.dev}/include";

      # standalone packaging script available directly on PATH in any shell
      packProject = pkgs.writeShellScriptBin "pack-project" ''
        set -e
        TARGET_DIR="''${1%/}"

        if [ -z "$TARGET_DIR" ]; then
          echo "Usage: pack-project <ProjectDirectory>"
          exit 1
        fi

        if [ ! -d "$TARGET_DIR" ]; then
          echo "Error: Directory '$TARGET_DIR' does not exist."
          exit 1
        fi

        mkdir -p zips
        BASE_NAME=$(basename "$TARGET_DIR")
        ZIP_NAME="zips/Kowal_Jakub_''${BASE_NAME}.zip"
        ROOT_IGNORE=".zipignore"

        echo "Packaging $TARGET_DIR -> $ZIP_NAME..."

        if [ -f "$ROOT_IGNORE" ]; then
          ${pkgs.zip}/bin/zip -r "$ZIP_NAME" "$TARGET_DIR" -x@"$ROOT_IGNORE"
        else
          ${pkgs.zip}/bin/zip -r "$ZIP_NAME" "$TARGET_DIR" -x "*/Resources/*" "*.tex" "*.aux" "*.log"
        fi

        echo "Testing archive integrity..."
        ${pkgs.unzip}/bin/unzip -t "$ZIP_NAME"
        echo "Done. Inspect with: unzip -l $ZIP_NAME"
      '';
    in {
      devShells.default = pkgs.mkShell {
        name = "nmp-lab-shell";

        buildInputs = [
          # build tools
          pkgs.gcc
          pkgs.gnumake
          pkgs.cmake
          pkgs.pkg-config

          # numerical libraries
          pkgs.openblas
          pkgs.fftw

          # python stack
          pythonEnv

          # utilities
          pkgs.git
          pkgs.nano
          pkgs.zip
          pkgs.unzip
          pkgs.rsync
          pkgs.openssh
          pkgs.sshfs
          pkgs.curl

          packProject
        ];

        # provide compiler include/lib paths without polluting global LD_LIBRARY_PATH
        NIX_CFLAGS_COMPILE = "-I${pkgs.openblas.dev}/include -I${pkgs.fftw.dev}/include";
        NIX_LDFLAGS = "-L${pkgs.openblas}/lib -L${pkgs.fftw}/lib";

        # Preprocessor and linker search paths for external tools, editors, and language servers
        CPLUS_INCLUDE_PATH = "${gccInclude}:${gccArchInclude}:${glibcInclude}:${openblasInclude}:${openblasInclude}/openblas:${fftwInclude}";
        C_INCLUDE_PATH = "${glibcInclude}:${openblasInclude}:${openblasInclude}/openblas:${fftwInclude}";
        LIBRARY_PATH = "${pkgs.openblas}/lib:${pkgs.fftw}/lib";

        shellHook = ''
          # prevent inherited library path overrides from interfering with Python
          unset LD_LIBRARY_PATH

          # Configure VS Code C/C++ IntelliSense & settings for Nix
          mkdir -p .vscode
          cat > .vscode/c_cpp_properties.json <<EOF
{
  "configurations": [
    {
      "name": "Nix GCC",
      "includePath": [
        "${gccInclude}",
        "${gccArchInclude}",
        "${gccInclude}/backward",
        "${glibcInclude}",
        "${openblasInclude}",
        "${openblasInclude}/openblas",
        "${fftwInclude}",
        "\''${workspaceFolder}/**",
        "\''${workspaceFolder}/include"
      ],
      "defines": [
        "STUDENT_WITH_BLAS"
      ],
      "compilerPath": "${pkgs.gcc}/bin/g++",
      "cStandard": "c17",
      "cppStandard": "c++17",
      "intelliSenseMode": "linux-gcc-x64"
    }
  ],
  "version": 4
}
EOF

          cat > .vscode/settings.json <<EOF
{
  "C_Cpp.default.compilerPath": "${pkgs.gcc}/bin/g++",
  "C_Cpp.default.cppStandard": "c++17",
  "C_Cpp.default.cStandard": "c17",
  "C_Cpp.default.includePath": [
    "${gccInclude}",
    "${gccArchInclude}",
    "${gccInclude}/backward",
    "${glibcInclude}",
    "${openblasInclude}",
    "${openblasInclude}/openblas",
    "${fftwInclude}",
    "\''${workspaceFolder}/**",
    "\''${workspaceFolder}/include"
  ],
  "C_Cpp.default.defines": [
    "STUDENT_WITH_BLAS"
  ],
  "C_Cpp.default.intelliSenseMode": "linux-gcc-x64"
}
EOF

          cat > .vscode/compile_flags.txt <<EOF
-xc++
-std=c++17
-Iinclude
-I${openblasInclude}
-I${openblasInclude}/openblas
-I${fftwInclude}
-DSTUDENT_WITH_BLAS
EOF

          # create course directory tree (0-indexed projects)
          mkdir -p setup Resources zips
          mkdir -p Project{00..15}/{src,data,plots,report}
          touch Project{00..15}/README.md

          # Propagate VS Code configurations and direnv activation to each Project directory
          for proj_dir in Project{00..15}; do
            if [ -d "$proj_dir" ]; then
              mkdir -p "$proj_dir/.vscode"
              cp -f .vscode/c_cpp_properties.json "$proj_dir/.vscode/c_cpp_properties.json"
              cp -f .vscode/settings.json "$proj_dir/.vscode/settings.json"
              cp -f .vscode/compile_flags.txt "$proj_dir/compile_flags.txt"
              if [ ! -f "$proj_dir/.envrc" ]; then
                echo "source_env ../.envrc" > "$proj_dir/.envrc"
              fi
            fi
          done

          # Helper function to unpack a lab zip without nested duplicate folders
          unpack_lab_zip() {
            local zip_file="$1"
            local target_name="$2"
            local dest_dir="Resources/$target_name"

            if [ -d "$dest_dir" ]; then
              return 0
            fi

            local extract_tmp
            extract_tmp=$(mktemp -d)
            ${pkgs.unzip}/bin/unzip -q -o "$zip_file" -d "$extract_tmp"

            mkdir -p "$dest_dir"
            if [ -d "$extract_tmp/$target_name" ]; then
              cp -a "$extract_tmp/$target_name"/. "$dest_dir/"
            else
              cp -a "$extract_tmp"/. "$dest_dir/"
            fi
            rm -rf "$extract_tmp"
          }

          # 1. Automatically fetch the next lab(s) from AGH server
          AGH_BASE_URL="https://home.agh.edu.pl/~khanpour"
          next_lab_idx=1
          while [ -d "Resources/nmpLab$(printf "%02d" "$next_lab_idx")" ]; do
            next_lab_idx=$((next_lab_idx + 1))
          done

          while true; do
            lab_str=$(printf "%02d" "$next_lab_idx")
            archive_url="$AGH_BASE_URL/nmpLab''${lab_str}.zip"
            dest_dir="Resources/nmpLab''${lab_str}"

            temp_zip=$(mktemp --suffix=.zip)
            if ${pkgs.curl}/bin/curl -fsSL --connect-timeout 4 --max-time 30 -o "$temp_zip" "$archive_url" 2>/dev/null; then
              echo "Fetched new lab archive: nmpLab''${lab_str}.zip from AGH."
              unpack_lab_zip "$temp_zip" "nmpLab''${lab_str}"
              rm -f "$temp_zip"
              next_lab_idx=$((next_lab_idx + 1))
            else
              rm -f "$temp_zip"
              break
            fi
          done

          # 2. Local fallback: unpack any missing zips from ~/Downloads
          DOWNLOADS_DIR="$HOME/Downloads"
          if [ -d "$DOWNLOADS_DIR" ]; then
            for zip_path in "$DOWNLOADS_DIR"/*Lab*.zip "$DOWNLOADS_DIR"/*lab*.zip; do
              if [ -f "$zip_path" ]; then
                zip_filename=$(basename "$zip_path")
                target_name="''${zip_filename%.zip}"
                dest_dir="Resources/$target_name"

                if [ ! -d "$dest_dir" ]; then
                  echo "Found new lab archive in Downloads: $zip_filename. Extracting to $dest_dir..."
                  unpack_lab_zip "$zip_path" "$target_name"
                fi
              fi
            done
          fi

          # 3. Symlink project resources from main resources dir to project specific dirs (offset by 1)
          # lab 1 -> Project00, lab 2 -> Project01, etc.
          # Clean up any stale or mismatched Resources symlinks
          for proj_dir in Project[0-9]*; do
            if [ -d "$proj_dir" ]; then
              p_num=$(echo "$proj_dir" | grep -o -E '[0-9]+' | head -n1)
              if [ -n "$p_num" ]; then
                expected_lab=$((10#$p_num + 1))
                expected_target="../Resources/nmpLab$(printf "%02d" "$expected_lab")"
                if [ -L "$proj_dir/Resources" ]; then
                  current_target=$(readlink "$proj_dir/Resources")
                  if [ "$current_target" != "$expected_target" ] || [ ! -d "$proj_dir/$current_target" ]; then
                    rm "$proj_dir/Resources"
                  fi
                fi
              fi
            fi
          done

          for res_path in Resources/*Lab[0-9]* Resources/*lab[0-9]*; do
            if [ -d "$res_path" ]; then
              res_name=$(basename "$res_path")
              lab_num=$(echo "$res_name" | grep -o -E '[0-9]+' | head -n1)

              if [ -n "$lab_num" ]; then
                proj_num=$((10#$lab_num - 1))
                proj_formatted=$(printf "%02d" "$proj_num")
                target_dir="Project$proj_formatted"

                if [ -d "$target_dir" ]; then
                  if [ -L "$target_dir/Resources" ]; then
                    rm "$target_dir/Resources"
                  fi
                  ln -sfn "../$res_path" "$target_dir/Resources"
                fi
              fi
            fi
          done

          # handles the case when spawned via nix develop
          if [ -z "$IN_NUSHELL" ] && [ -z "$DIRENV_IN_ENVRC" ] && [ -t 0 ] && command -v nu >/dev/null 2>&1; then
            export IN_NUSHELL=1
            exec nu
          fi
        '';
      };
    });
}
