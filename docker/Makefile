all:
	@echo "Build targets: "
	@echo "build-docker		Build an image for building tools with Docker"
	@echo "build			Build project assets from within Docker container"
	@echo "run			    Run a shell using the above image with Docker"

IMAGE_NAME=ch554_sdcc


COMMON = run \
		--rm \
		--mount type=bind,source="`pwd`/..",target=/build \
		-w /build \

build-docker:
	docker build -t ${IMAGE_NAME} -f Dockerfile .

build:
	docker ${COMMON} \
		-it ${IMAGE_NAME} \
		/bin/bash -c "cd examples; make clean; make"
		
run:
	docker ${COMMON} \
		-it ${IMAGE_NAME} \
		/bin/bash