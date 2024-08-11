# For more information, please refer to https://aka.ms/vscode-docker-python
FROM python:3

# make bash default for exec
RUN ln -sf /bin/bash /bin/sh

RUN apt-get update && apt-get -y install cmake python3-dev 

ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

# install nvm + node
ENV NODE_VERSION=22.2.0
RUN apt install -y curl
RUN curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
ENV NVM_DIR=/root/.nvm
RUN . "$NVM_DIR/nvm.sh" && nvm install ${NODE_VERSION}
RUN . "$NVM_DIR/nvm.sh" && nvm use v${NODE_VERSION}
RUN . "$NVM_DIR/nvm.sh" && nvm alias default v${NODE_VERSION}
ENV PATH="/root/.nvm/versions/node/v${NODE_VERSION}/bin/:${PATH}"

# Install pip requirements
COPY docker/pydev/requirements.txt .
RUN python -m pip install -r requirements.txt
RUN jupyter labextension disable "@jupyterlab/apputils-extension:announcements"

WORKDIR /app
COPY . /app

# build python bindings
RUN cmake . -B bin -DSENSOR_BIND=ON -DCMAKE_CXX_FLAGS="-fPIC"
RUN cmake --build bin
ENV PYTHONPATH "${PYTHONPATH}:/app/bin/bind/"

# run as non-root
RUN adduser -u 5678 --disabled-password --gecos "" appuser && chown -R appuser /app
USER appuser
EXPOSE 8888

# start the jupyter server
# output will contain the uri + auth token!
ENTRYPOINT ["jupyter", "lab","--ip=0.0.0.0","--allow-root", "--no-browser", "-y",  \
  "--ServerApp.root_dir='/home/appuser/py'", \
  "--FileContentsManager.preferred_dir='ipynb'"]