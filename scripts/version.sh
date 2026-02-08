#!/bin/bash
set -e

# Professional Versioning Script for Flux
# Usage: ./scripts/version.sh 0.1.1

NEW_VERSION=$1

if [ -z "$NEW_VERSION" ]; then
    echo "Usage: ./scripts/version.sh <major.minor.patch>"
    exit 1
fi

# 1. Update CMakeLists.txt
sed -i "s/project(Flux VERSION [0-9.]*)/project(Flux VERSION $NEW_VERSION)/" CMakeLists.txt

# 2. Update package.json
npm version $NEW_VERSION --no-git-tag-version

# 3. Update editors/vscode/package.json
(cd editors/vscode && npm version $NEW_VERSION --no-git-tag-version)

# 4. Update tools/flux/flux.rc (Windows Resource)
# Convert 0.1.1 to 0,1,1,0
RC_VERSION=$(echo $NEW_VERSION | sed 's/\./,/g'),0
sed -i "s/FILEVERSION     [0-9, ]*/FILEVERSION     $RC_VERSION/" tools/flux/flux.rc
sed -i "s/PRODUCTVERSION  [0-9, ]*/PRODUCTVERSION  $RC_VERSION/" tools/flux/flux.rc
sed -i "s/VALUE \"FileVersion\",      \"[0-9.]*\"/VALUE \"FileVersion\",      \"$NEW_VERSION\"/" tools/flux/flux.rc
sed -i "s/VALUE \"ProductVersion\",   \"[0-9.]*\"/VALUE \"ProductVersion\",   \"$NEW_VERSION\"/" tools/flux/flux.rc

echo "==> Flux project version updated to $NEW_VERSION across all components."
