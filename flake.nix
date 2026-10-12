{
  description = "Numerical Methods in Physics (NMP) lab development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    codesnippet-src = {
      url = "github:justkowal/codesnippet";
      flake = false;
    };
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    codesnippet-src,
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
          pygments
        ]);

      # Include and library discovery for compilers, LSPs, and VS Code
      gccVersion = pkgs.gcc.version;
      gccInclude = "${pkgs.gcc.cc}/include/c++/${gccVersion}";
      gccArchInclude = "${pkgs.gcc.cc}/include/c++/${gccVersion}/${pkgs.stdenv.targetPlatform.config}";
      glibcInclude = "${pkgs.glibc.dev}/include";
      openblasInclude = "${pkgs.openblas.dev}/include";
      fftwInclude = "${pkgs.fftw.dev}/include";

      # Custom LaTeX code extraction package
      codesnippetPkg = pkgs.stdenvNoCC.mkDerivation {
        pname = "codesnippet";
        version = "1.0";
        src = codesnippet-src;
        dontUnpack = true;
        tlType = "run";
        installPhase = ''
          mkdir -p $out/tex/latex/codesnippet
          cp $src/codesnippet.sty $src/extract.py $out/tex/latex/codesnippet/
          mkdir -p $out/bin
          cp $src/extract.py $out/bin/codesnippet-extract
          chmod +x $out/bin/codesnippet-extract $out/tex/latex/codesnippet/extract.py
        '';
      };

      # Full LaTeX environment with minted, modern packages, and codesnippet
      texliveEnv = pkgs.texliveMedium.withPackages (ps: with ps; [
        minted
        fvextra
        fancyvrb
        lineno
        framed
        float
        xcolor
        kvoptions
        pgf
        catchfile
        latexmk
        booktabs
        caption
        microtype
        hyperref
        amsmath
        geometry
        titlesec
        enumitem
        cleveref
        codesnippetPkg
      ]);

      # Helper script to sync local companion extension and install recommended IDE extensions
      setupIdeExtensions = pkgs.writeShellScriptBin "setup-ide-extensions" ''
        set -e
        REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
        EXT_SRC="$REPO_ROOT/setup/codesnippet-companion"

        # 1. Sync custom codesnippet companion extension to VS Code and Antigravity IDE
        if [ -d "$EXT_SRC" ]; then
          for ext_dir in "$HOME/.vscode/extensions" "$HOME/.antigravity-ide/extensions"; do
            mkdir -p "$ext_dir/nmp.codesnippet-companion-1.0.0"
            cp -rf "$EXT_SRC"/* "$ext_dir/nmp.codesnippet-companion-1.0.0/" 2>/dev/null || true
          done
        fi

        # 2. Curated extensions strictly relevant to Numerical Methods in Physics
        EXTENSIONS=(
          "ms-vscode.cpptools"
          "llvm-vs-code-extensions.vscode-clangd"
          "ms-vscode.cmake-tools"
          "ms-vscode.makefile-tools"
          "ms-python.python"
          "ms-python.debugpy"
          "James-Yu.latex-workshop"
          "tomoki1207.pdf"
          "mkhl.direnv"
          "jnoortheen.nix-ide"
          "AaronKazah.copy-reference-jetbrains"
          "sandipchitale.vscode-fqn"
        )

        for ide in code antigravity-ide; do
          if command -v "$ide" >/dev/null 2>&1; then
            INSTALLED="$("$ide" --list-extensions 2>/dev/null || true)"
            for ext in "''${EXTENSIONS[@]}"; do
              if ! echo "$INSTALLED" | grep -qi "^$ext\$"; then
                echo "Installing $ext into $ide..."
                "$ide" --install-extension "$ext" >/dev/null 2>&1 || true
              fi
            done
          fi
        done

        # 3. Share marketplace extensions across VS Code & Antigravity IDE
        for ext in "$HOME/.vscode/extensions"/*copy-reference* "$HOME/.vscode/extensions"/*vscode-fqn*; do
          if [ -d "$ext" ]; then
            base_name=$(basename "$ext")
            if [ ! -d "$HOME/.antigravity-ide/extensions/$base_name" ]; then
              cp -rf "$ext" "$HOME/.antigravity-ide/extensions/" 2>/dev/null || true
            fi
          fi
        done
      '';

      # standalone packaging script available directly on PATH in any shell
      packProject = pkgs.writeShellScriptBin "pack-project" ''
        set -e
        REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"

        INPUT_ARG="''${1%/}"

        # Resolve target directory
        TARGET_DIR=""
        if [ -n "$INPUT_ARG" ]; then
          if [ "$INPUT_ARG" = "repo" ] || { [ "$INPUT_ARG" = "." ] && [ "$PWD" = "$REPO_ROOT" ]; }; then
            TARGET_DIR="$REPO_ROOT"
          elif [ -d "$INPUT_ARG" ]; then
            TARGET_DIR="$(cd "$INPUT_ARG" && pwd)"
          elif [ -d "$REPO_ROOT/$INPUT_ARG" ]; then
            TARGET_DIR="$REPO_ROOT/$INPUT_ARG"
          else
            # Try interpreting as numeric index or ProjectXX
            NUM=$(echo "$INPUT_ARG" | grep -o -E '[0-9]+' | head -n1)
            if [ -n "$NUM" ]; then
              PROJ_CANDIDATE="$REPO_ROOT/Project$(printf "%02d" "$((10#$NUM))")"
              if [ -d "$PROJ_CANDIDATE" ]; then
                TARGET_DIR="$PROJ_CANDIDATE"
              fi
            fi
          fi
        else
          # If no argument given, check if current directory is a project directory or repo root
          CURR_NAME=$(basename "$PWD")
          if echo "$CURR_NAME" | grep -q -E '^Project[0-9]+'; then
            TARGET_DIR="$PWD"
          elif [ "$PWD" = "$REPO_ROOT" ]; then
            TARGET_DIR="$REPO_ROOT"
          fi
        fi

        if [ -z "$TARGET_DIR" ] || [ ! -d "$TARGET_DIR" ]; then
          echo "Usage: pack-project [ProjectDirectory|Index|repo]"
          echo "Examples: pack-project Project01, pack-project 01, or pack-project repo"
          exit 1
        fi

        if [ "$TARGET_DIR" = "$REPO_ROOT" ]; then
          PACKAGE_NAME="Kowal_Jakub_NMP"
          BASE_NAME="Repository"
        else
          BASE_NAME=$(basename "$TARGET_DIR")
          PACKAGE_NAME="Kowal_Jakub_''${BASE_NAME}"
        fi

        ZIPS_DIR="$REPO_ROOT/zips"
        ZIP_NAME="$ZIPS_DIR/''${PACKAGE_NAME}.zip"
        ROOT_IGNORE="$REPO_ROOT/.zipignore"

        mkdir -p "$ZIPS_DIR"
        rm -f "$ZIP_NAME"

        echo "Packaging $BASE_NAME as $PACKAGE_NAME -> $ZIP_NAME..."

        # Stage with the submission package name so internal directory matches zip name
        STAGE_DIR=$(mktemp -d)
        DEST_DIR="$STAGE_DIR/$PACKAGE_NAME"
        mkdir -p "$DEST_DIR"
        cp -a "$TARGET_DIR/." "$DEST_DIR/"

        # Rename bundled.nix to flake.nix (or copy from repo root) and hook up direnv
        if [ -f "$DEST_DIR/bundled.nix" ]; then
          mv -f "$DEST_DIR/bundled.nix" "$DEST_DIR/flake.nix"
        elif [ -f "$REPO_ROOT/bundled.nix" ]; then
          cp -f "$REPO_ROOT/bundled.nix" "$DEST_DIR/flake.nix"
        fi

        if [ -f "$REPO_ROOT/flake.lock" ] && [ ! -f "$DEST_DIR/flake.lock" ]; then
          cp -f "$REPO_ROOT/flake.lock" "$DEST_DIR/flake.lock"
        fi

        echo "use flake" > "$DEST_DIR/.envrc"

        # Clean up symlinks, build artifacts, git and caches before archiving
        rm -rf "$DEST_DIR/Resources" "$DEST_DIR/.git" "$DEST_DIR"/build "$DEST_DIR"/*build* "$DEST_DIR"/bin "$DEST_DIR"/.cache "$DEST_DIR"/.vscode "$DEST_DIR"/.direnv "$DEST_DIR"/zips
        rm -f "$DEST_DIR/compile_flags.txt" "$DEST_DIR/compile_commands.json"

        (
          cd "$STAGE_DIR"
          if [ -f "$ROOT_IGNORE" ]; then
            ${pkgs.zip}/bin/zip -r "$ZIP_NAME" "$PACKAGE_NAME" -x@"$ROOT_IGNORE"
          else
            ${pkgs.zip}/bin/zip -r "$ZIP_NAME" "$PACKAGE_NAME"
          fi
        )
        rm -rf "$STAGE_DIR"

        echo "Testing archive integrity..."
        ${pkgs.unzip}/bin/unzip -t "$ZIP_NAME"
        echo "Done. Package created at $ZIP_NAME"
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

          # latex stack & tools
          texliveEnv
          pkgs.ghostscript
          pkgs.which

          packProject
          setupIdeExtensions
        ];

        # provide compiler include/lib paths without polluting global LD_LIBRARY_PATH
        NIX_CFLAGS_COMPILE = "-I${pkgs.openblas.dev}/include -I${pkgs.openblas.dev}/include/openblas -I${pkgs.fftw.dev}/include";
        NIX_LDFLAGS = "-L${pkgs.glibc}/lib -L${pkgs.openblas}/lib -L${pkgs.fftw}/lib";

        # Preprocessor and linker search paths for external tools, editors, and language servers
        CPLUS_INCLUDE_PATH = "${gccInclude}:${gccArchInclude}:${glibcInclude}:${openblasInclude}:${openblasInclude}/openblas:${fftwInclude}";
        C_INCLUDE_PATH = "${glibcInclude}:${openblasInclude}:${openblasInclude}/openblas:${fftwInclude}";
        LIBRARY_PATH = "${pkgs.glibc}/lib:${pkgs.openblas}/lib:${pkgs.fftw}/lib";
        CMAKE_LIBRARY_PATH = "${pkgs.glibc}/lib:${pkgs.openblas}/lib:${pkgs.fftw}/lib";
        CMAKE_INCLUDE_PATH = "${openblasInclude}:${openblasInclude}/openblas:${fftwInclude}";

        shellHook = ''
          # prevent inherited library path overrides from interfering with Python/build
          unset LD_LIBRARY_PATH
          unset NIX_CFLAGS_LINK

          # Locate repo root accurately even if invoked from subdirectories
          REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"

          # TeX environment paths: make custom codesnippet package and scripts immediately discoverable
          export TEXINPUTS=".:$REPO_ROOT/codesnippet//:$TEXINPUTS"
          export PATH="$REPO_ROOT/codesnippet:$PATH"

          # Sync custom codesnippet companion extension for VS Code & Antigravity IDE
          if [ -d "$REPO_ROOT/setup/codesnippet-companion" ]; then
            for ext_dir in "$HOME/.vscode/extensions" "$HOME/.antigravity-ide/extensions"; do
              mkdir -p "$ext_dir/nmp.codesnippet-companion-1.0.0"
              cp -rf "$REPO_ROOT/setup/codesnippet-companion"/* "$ext_dir/nmp.codesnippet-companion-1.0.0/" 2>/dev/null || true
            done
          fi

          # Configure VS Code & Antigravity workspace recommendations & settings at repo root
          mkdir -p "$REPO_ROOT/.vscode"

          cat > "$REPO_ROOT/.vscode/extensions.json" <<EOF
{
  "recommendations": [
    "ms-vscode.cpptools",
    "llvm-vs-code-extensions.vscode-clangd",
    "ms-vscode.cmake-tools",
    "ms-vscode.makefile-tools",
    "ms-python.python",
    "ms-python.debugpy",
    "James-Yu.latex-workshop",
    "tomoki1207.pdf",
    "mkhl.direnv",
    "jnoortheen.nix-ide",
    "AaronKazah.copy-reference-jetbrains",
    "sandipchitale.vscode-fqn"
  ]
}
EOF

          cat > "$REPO_ROOT/.vscode/c_cpp_properties.json" <<EOF
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

          cat > "$REPO_ROOT/.vscode/settings.json" <<EOF
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
  "C_Cpp.default.intelliSenseMode": "linux-gcc-x64",
  "latex-workshop.latex.tools": [
    {
      "name": "pdflatex",
      "command": "pdflatex",
      "args": [
        "-synctex=1",
        "-interaction=nonstopmode",
        "-file-line-error",
        "-shell-escape",
        "%DOC%"
      ]
    },
    {
      "name": "latexmk",
      "command": "latexmk",
      "args": [
        "-synctex=1",
        "-interaction=nonstopmode",
        "-file-line-error",
        "-pdf",
        "-shell-escape",
        "-outdir=%OUTDIR%",
        "%DOC%"
      ]
    }
  ],
  "latex-workshop.latex.recipes": [
    {
      "name": "pdflatex (shell-escape)",
      "tools": [
        "pdflatex"
      ]
    },
    {
      "name": "latexmk (shell-escape)",
      "tools": [
        "latexmk"
      ]
    }
  ],
  "latex-workshop.view.pdf.viewer": "tab",
  "latex-workshop.formatting.latex": "latexindent",
  "[latex]": {
    "editor.defaultFormatter": "James-Yu.latex-workshop"
  }
}
EOF

          cat > "$REPO_ROOT/.vscode/compile_flags.txt" <<EOF
-xc++
-std=c++17
-Iinclude
-I${openblasInclude}
-I${openblasInclude}/openblas
-I${fftwInclude}
-DSTUDENT_WITH_BLAS
EOF

          # Ensure base workspace directories exist
          mkdir -p "$REPO_ROOT/setup" "$REPO_ROOT/Resources" "$REPO_ROOT/zips"

          # Helper function to unpack a lab zip cleanly into Resources/<target_name>
          unpack_lab_zip() {
            local zip_file="$1"
            local target_name="$2"
            local dest_dir="$REPO_ROOT/Resources/$target_name"

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
          while [ -d "$REPO_ROOT/Resources/nmpLab$(printf "%02d" "$next_lab_idx")" ]; do
            next_lab_idx=$((next_lab_idx + 1))
          done

          while true; do
            lab_str=$(printf "%02d" "$next_lab_idx")
            archive_url="$AGH_BASE_URL/nmpLab''${lab_str}.zip"

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

          # 2. Local fallback: unpack any missing zips from ~/Downloads or repo zips/
          DOWNLOADS_DIR="$HOME/Downloads"
          if [ -d "$DOWNLOADS_DIR" ]; then
            for zip_path in "$DOWNLOADS_DIR"/*Lab*.zip "$DOWNLOADS_DIR"/*lab*.zip; do
              if [ -f "$zip_path" ]; then
                zip_filename=$(basename "$zip_path")
                target_name="''${zip_filename%.zip}"
                dest_dir="$REPO_ROOT/Resources/$target_name"

                if [ ! -d "$dest_dir" ]; then
                  echo "Found new lab archive in Downloads: $zip_filename. Extracting to $dest_dir..."
                  unpack_lab_zip "$zip_path" "$target_name"
                fi
              fi
            done
          fi

          if [ -d "$REPO_ROOT/zips" ]; then
            for zip_path in "$REPO_ROOT/zips"/*Lab*.zip "$REPO_ROOT/zips"/*lab*.zip; do
              if [ -f "$zip_path" ]; then
                zip_filename=$(basename "$zip_path")
                target_name="''${zip_filename%.zip}"
                dest_dir="$REPO_ROOT/Resources/$target_name"

                if [ ! -d "$dest_dir" ]; then
                  echo "Found new lab archive in zips: $zip_filename. Extracting to $dest_dir..."
                  unpack_lab_zip "$zip_path" "$target_name"
                fi
              fi
            done
          fi

          # 3. Only create project folders for projects that have accessible zip packages (in Resources)
          # offset by 1: nmpLab01 -> Project00, nmpLab02 -> Project01, etc.
          for res_path in "$REPO_ROOT"/Resources/*Lab[0-9]* "$REPO_ROOT"/Resources/*lab[0-9]*; do
            if [ -d "$res_path" ]; then
              res_name=$(basename "$res_path")
              lab_num=$(echo "$res_name" | grep -o -E '[0-9]+' | head -n1)

              if [ -n "$lab_num" ]; then
                proj_num=$((10#$lab_num - 1))
                proj_formatted=$(printf "%02d" "$proj_num")
                target_dir="$REPO_ROOT/Project$proj_formatted"

                # Create project scaffold if it doesn't already exist
                mkdir -p "$target_dir"/{src,data,plots,report}
                if [ ! -f "$target_dir/README.md" ]; then
                  touch "$target_dir/README.md"
                fi
                if [ ! -f "$target_dir/CMakeLists.txt" ] && [ -f "$REPO_ROOT/setup/CMakeLists.template.txt" ]; then
                  cp "$REPO_ROOT/setup/CMakeLists.template.txt" "$target_dir/CMakeLists.txt"
                fi

                # Propagate configurations without touching user code
                mkdir -p "$target_dir/.vscode"
                cp -f "$REPO_ROOT/.vscode/c_cpp_properties.json" "$target_dir/.vscode/c_cpp_properties.json"
                cp -f "$REPO_ROOT/.vscode/settings.json" "$target_dir/.vscode/settings.json"
                cp -f "$REPO_ROOT/.vscode/extensions.json" "$target_dir/.vscode/extensions.json"
                if [ ! -f "$target_dir/compile_flags.txt" ]; then
                  cp -f "$REPO_ROOT/.vscode/compile_flags.txt" "$target_dir/compile_flags.txt"
                fi

                # Symlink Resources to project directory (never touch real directories/files)
                expected_target="../Resources/$res_name"
                if [ -L "$target_dir/Resources" ]; then
                  current_target=$(readlink "$target_dir/Resources")
                  if [ "$current_target" != "$expected_target" ]; then
                    rm "$target_dir/Resources"
                    ln -s "$expected_target" "$target_dir/Resources"
                  fi
                elif [ ! -e "$target_dir/Resources" ]; then
                  ln -s "$expected_target" "$target_dir/Resources"
                fi
              fi
            fi
          done

          # Auto setup IDE extensions in background (one-time cached, fully detached)
          if [ ! -f "$HOME/.cache/nmp-ide-setup-done" ] && command -v setup-ide-extensions >/dev/null 2>&1; then
            mkdir -p "$HOME/.cache"
            touch "$HOME/.cache/nmp-ide-setup-done"
            ( nohup setup-ide-extensions </dev/null >/dev/null 2>&1 & )
          fi

          # handles the case when spawned via nix develop
          if [ -z "$IN_NUSHELL" ] && [ -z "$DIRENV_IN_ENVRC" ] && [ -t 0 ] && command -v nu >/dev/null 2>&1; then
            export IN_NUSHELL=1
            exec nu
          fi
        '';
      };
    });
}
