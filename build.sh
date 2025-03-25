#!/usr/bin/env bash

# ./build_and_push.sh [version]

IMAGE_NAME="jkingsman/qotd-appliance"
DEFAULT_TAG="latest"
VERSION_TAG=""

if [ $# -eq 1 ]; then
    VERSION_TAG=$1
    echo "Building with tags: ${DEFAULT_TAG} and ${VERSION_TAG}"
else
    echo "No version specified. Building with tag: ${DEFAULT_TAG} only"
fi

echo "Building Docker image: ${IMAGE_NAME}:${DEFAULT_TAG}"
docker build -t ${IMAGE_NAME}:${DEFAULT_TAG} .

if [ $? -eq 0 ]; then
    echo "Docker image built successfully."
else
    echo "Docker image build failed. Exiting."
    exit 1
fi

if [ -n "${VERSION_TAG}" ]; then
    echo "Tagging image as: ${IMAGE_NAME}:${VERSION_TAG}"
    docker tag ${IMAGE_NAME}:${DEFAULT_TAG} ${IMAGE_NAME}:${VERSION_TAG}
fi

echo "Pushing image to Docker Hub: ${IMAGE_NAME}:${DEFAULT_TAG}"
docker push ${IMAGE_NAME}:${DEFAULT_TAG}

if [ -n "${VERSION_TAG}" ]; then
    echo "Pushing image to Docker Hub: ${IMAGE_NAME}:${VERSION_TAG}"
    docker push ${IMAGE_NAME}:${VERSION_TAG}
fi

echo "Process completed."
