FROM python:3.11-alpine

WORKDIR /app
RUN pip install --no-cache-dir fastapi uvicorn
COPY app.py ./
EXPOSE 8000
ENV PORT=8000

CMD ["uvicorn", "app:app", "--host", "0.0.0.0"]