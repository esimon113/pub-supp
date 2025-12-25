import socket

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(("127.0.0.1", 8080))
    server_socket.listen()
    print("Server listening on 127.0.0.1:8080")

    while True:
        client_connection, client_address = server_socket.accept()

        with client_connection:
            print(f"Accepted connection from {client_address}")

            data = client_connection.recv(1024)

            if not data:
                break

            print(f"Received data: {data.decode()}")

            client_connection.sendall(b"Hello from the server!")
