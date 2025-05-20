import sys
import re


def xor_encode(binary_data, key):
    """
    XOR encodes the given binary data with the provided key.

    Args:
        binary_data (bytes): The input binary data.
        key (bytes): The key used to XOR encode the binary data.

    Returns:
        bytes: The XOR-encoded binary data.
    """
    encoded_bytes = bytearray()
    for i, byte in enumerate(binary_data):
        encoded_bytes.append(byte ^ key[i % len(key)])
    return encoded_bytes


def parse_key(key_input):
    """
    Parses the XOR key in the format "0xXX, 0xYY, ..." into raw bytes.

    Args:
        key_input (str): The XOR key in the specified format.

    Returns:
        bytes: A bytes object containing the parsed key values.
    """
    # Extract all "0xXX" hex values using regex
    matches = re.findall(r'0x([0-9a-fA-F]{2})', key_input)
    if not matches:
        raise ValueError("Invalid key format. Expected format: 0xXX, 0xYY, ...")
    return bytes(int(match, 16) for match in matches)


def format_as_hexcode(encoded_bytes):
    """
    Formats the output bytes into the input format "0xXX, 0xYY, ...".

    Args:
        encoded_bytes (bytes): The XOR-encoded bytes.

    Returns:
        str: The formatted hexcode string.
    """
    formatted = ", ".join(f"0x{byte:02x}" for byte in encoded_bytes)
    # Group into lines of 8 bytes for readability
    return "\n  ".join([formatted[i:i+48] for i in range(0, len(formatted), 48)])


if __name__ == "__main__":
    # Check if the user provided the file path and key as arguments
    if len(sys.argv) != 3:
        print("Usage: python xor_encoder_elf64.py <file_path> <key>")
        print("Example key format: 0x67,0x89,0xab")
        sys.exit(1)

    file_path = sys.argv[1]
    key_input = sys.argv[2]

    try:
        # Read the binary data from the file
        with open(file_path, "rb") as binary_file:
            binary_data = binary_file.read()

        # Parse the XOR key into raw bytes
        key = parse_key(key_input)

        # Perform XOR encoding
        encoded_bytes = xor_encode(binary_data, key)

        # Format the encoded bytes as the input format
        encoded_hexcode = format_as_hexcode(encoded_bytes)
        print(f"{encoded_hexcode}")

    except FileNotFoundError:
        print(f"File not found: {file_path}")
        sys.exit(1)

    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)
