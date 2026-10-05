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

      pythonEnv = pkgs.python3.withPackages (ps:
        with ps; [
          numpy
          scipy
          matplotlib
        ]);

      # Standalone packaging script available directly on PATH in any shell
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

        ZIP_NAME="''${TARGET_DIR}.zip"
        ROOT_IGNORE=".zipignore"

        echo "Packaging $TARGET_DIR ->$ZIP_NAME..."

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
          # C/C++ toolchain & build tools (LM3)
          pkgs.gcc
          pkgs.gnumake
          pkgs.cmake
          pkgs.pkg-config

          # Numerical libraries (LM5)
          pkgs.lapack-reference
          pkgs.openblas
          pkgs.fftw

          # Python stack for plotting & validation (LM7)
          pythonEnv

          # Workflow, transfer & archive utilities (LM3, LM10)
          pkgs.git
          pkgs.nano
          pkgs.zip
          pkgs.unzip
          pkgs.rsync
          pkgs.openssh
          pkgs.sshfs

          # Custom CLI utilities
          packProject
        ];

        # Expose shared libraries for dynamically linked test binaries
        LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
          pkgs.lapack-reference
          pkgs.openblas
          pkgs.fftw
        ];

        shellHook = ''
          # create course directory tree
          mkdir -p setup Resources
          mkdir -p Kowal_Jakub_Project{01..15}/{src,data,plots,report}
          touch Kowal_Jakub_Project{01..15}/README.md

          # unpack any missing zips from downloads to resources folder at root
          DOWNLOADS_DIR="$HOME/Downloads"
          if [ -d "$DOWNLOADS_DIR" ]; then
            for zip_path in "$DOWNLOADS_DIR"/*Lab*.zip "$DOWNLOADS_DIR"/*lab*.zip; do
              if [ -f "$zip_path" ]; then
                zip_filename=$(basename "$zip_path")
                target_name="''${zip_filename%.zip}"
                dest_dir="Resources/$target_name"

                if [ ! -d "$dest_dir" ]; then
                  echo "Found new lab archive: $zip_filename. Extracting to $dest_dir..."
                  mkdir -p "$dest_dir"
                  unzip -q -o "$zip_path" -d "$dest_dir"
                fi
              fi
            done
          fi

          # symlink project resources from main resources dir to project specific dirs
          for res_path in Resources/*Lab[0-9]* Resources/*lab[0-9]*; do
            if [ -d "$res_path" ]; then
              res_name=$(basename "$res_path")
              num=$(echo "$res_name" | grep -o -E '[0-9]+' | head -n1)

              if [ -n "$num" ]; then
                num_formatted=$(printf "%02d" "$((10#$num))")
                target_dir="Kowal_Jakub_Project$num_formatted"

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
          if [ -z "$IN_NUSHELL" ] && command -v nu >/dev/null 2>&1; then
            export IN_NUSHELL=1
            exec nu
          fi
        '';
      };
    });
}
