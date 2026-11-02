import subprocess

PORT = "/dev/ttyACM0"
FQBN = "arduino:avr:mega"
SKETCH_PATH = ("test_arduino/test_arduino.ino")

command = ["arduino-cli", "compile", "--upload", "-p", PORT, "--fqbn", FQBN, SKETCH_PATH]

print("compilation et televersment en cours...")


try:
    results = subprocess.run(command, capture_output=True, text=True, check = True)

    print("Succès !")
    print(results.stdout)

except subprocess.CalledProcessError as e:
    print("Erreur : ")
    print(e.stderr)
