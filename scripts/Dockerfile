FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates curl xz-utils make ninja-build git \
    gcc-multilib libc6-i386 lib32stdc++6 binutils-mips-linux-gnu \
    python3 python3-pip python3-venv libyaml-dev \
    && rm -rf /var/lib/apt/lists/*

# scripts/linux.sh supplies a small context, including the local compiler when
# available. A fresh checkout obtains it while building the image.
COPY requirements.txt /tmp/sotc-requirements.txt
RUN python3 -m pip install --no-cache-dir -r /tmp/sotc-requirements.txt
COPY compiler/ /opt/ee-gcc2.96/
RUN if [ ! -f /opt/ee-gcc2.96/bin/ee-gcc ]; then \
        curl --fail --location --retry 3 \
            https://github.com/decompme/compilers/releases/download/compilers/ee-gcc2.96.tar.xz \
            --output /tmp/ee-gcc2.96.tar.xz \
        && tar -xf /tmp/ee-gcc2.96.tar.xz -C /opt/ee-gcc2.96 \
        && rm /tmp/ee-gcc2.96.tar.xz; \
    fi \
    && chmod -R a+rx /opt/ee-gcc2.96

WORKDIR /work
CMD ["bash"]
