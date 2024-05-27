# For more information, please refer to https://aka.ms/vscode-docker-python
FROM python:3

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

WORKDIR /app
COPY . /app

# build python bindings
RUN cmake . -B bin
RUN cmake --build bin

# Creates a non-root user with an explicit UID and adds permission to access the /app folder
# For more info, please refer to https://aka.ms/vscode-docker-python-configure-containers
RUN adduser -u 5678 --disabled-password --gecos "" appuser && chown -R appuser /app
USER appuser

EXPOSE 8888

# RUN jupyter labextension disable "@jupyterlab/apputils-extension:announcements"
# ENTRYPOINT ["jupyter", "lab","--ip=0.0.0.0","--allow-root", "--no-browser", "-y", "--ServerApp.root_dir='/home/appuser/py'"]