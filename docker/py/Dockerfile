# For more information, please refer to https://aka.ms/vscode-docker-python
FROM python:3

# make bash default for exec
RUN ln -sf /bin/bash /bin/sh

RUN apt-get update && apt-get -y install cmake python3-dev 

ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

# Install pip requirements
COPY docker/py/requirements.txt .
RUN python -m pip install -r requirements.txt

WORKDIR /app
COPY . /app

# build python bindings
RUN cmake . -B bin -DSENSOR_BIND=ON
RUN cmake --build bin
ENV PYTHONPATH "${PYTHONPATH}:/app/bin/bind/"

# run as non-root
RUN adduser -u 5678 --disabled-password --gecos "" appuser && chown -R appuser /app
USER appuser
