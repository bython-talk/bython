FROM ghcr.io/bython-talk/bython-base AS bython-dev
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -q -y update
RUN apt-get -q -y install --no-install-recommends \
    ninja-build gdb-multiarch cppcheck clang-format-16 clangd \
    tig git vim openssh-client \
    zsh fzf \
    && apt-get clean all

# Uses "Spaceship" theme with some customization. Uses some bundled plugins and installs some more from github
RUN sh -c "$(wget -O- https://github.com/deluan/zsh-in-docker/releases/download/v1.1.5/zsh-in-docker.sh)" -- \
    -t robbyrussell \
    -p git \
    -p https://github.com/zsh-users/zsh-autosuggestions \
    -p https://github.com/zsh-users/zsh-completions
RUN chsh -s /usr/bin/zsh

RUN update-alternatives \
    --install /usr/bin/clang          clang         /usr/bin/clang-16 160 \
    --slave   /usr/bin/clang++        clang++       /usr/bin/clang++-16 \
    --slave   /usr/bin/clang-format   clang-format  /usr/bin/clang-format-16 \
    --slave   /usr/bin/lldb           lldb          /usr/bin/lldb-16

RUN update-alternatives \
    --install /usr/bin/c++      c++       /usr/bin/clang++  160
RUN update-alternatives \
    --install /usr/bin/cc       cc        /usr/bin/clang    160

RUN update-alternatives \
    --install /usr/bin/editor   editor    /usr/bin/vim      100
RUN git config --global commit.verbose true

RUN git config --global user.name "Bython Enjoyer"
RUN git config --global user.email "enjoyer@bython.com"

ENTRYPOINT [ "zsh" ]