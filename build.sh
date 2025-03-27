#!/usr/bin/env bash

# ./build.sh [version]

if [ $# -eq 0 ]; then
  echo "Error: Version argument is required"
  echo "Usage: $0 <version>"
  echo "Example: $0 1.0.0"
  exit 1
fi

# Set the version from command line argument
VERSION="$1"

# Build both images
echo "Building qotd_8ball image..."
docker buildx build --platform linux/arm/v7,linux/arm64/v8,linux/amd64 --target qotd_8ball -t jkingsman/qotd-appliance:latest -t jkingsman/qotd-appliance:8ball-${VERSION} -t jkingsman/qotd-appliance:8ball-latest .

echo "Building qotd_fortune_cowsay image..."
docker buildx build --platform linux/arm/v7,linux/arm64/v8,linux/amd64 --target qotd_fortune_cowsay -t jkingsman/qotd-appliance:fortune-cowsay-${VERSION} -t jkingsman/qotd-appliance:fortune-cowsay-latest .

# Push all tags to Docker Hub
echo "Pushing all images to Docker Hub..."
docker push jkingsman/qotd-appliance:latest
docker push jkingsman/qotd-appliance:8ball-${VERSION}
docker push jkingsman/qotd-appliance:8ball-latest
docker push jkingsman/qotd-appliance:fortune-cowsay-${VERSION}
docker push jkingsman/qotd-appliance:fortune-cowsay-latest

echo "All images built and pushed successfully!"
