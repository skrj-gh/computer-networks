import matplotlib.pyplot as plt
import numpy as np

def load_data(filename):
    time = []
    value = []
    with open(filename, 'r') as f:
        for line in f:
            t, v = map(float, line.strip().split())
            time.append(t)
            value.append(v)
    return np.array(time), np.array(value)

# Create figure with three subplots
plt.figure(figsize=(15, 10))

# Plot Throughput
plt.subplot(3, 1, 1)
time, throughput = load_data('throughput.dat')
plt.plot(time, throughput/ 1e6)  # Convert to Mbps
plt.title('Network Throughput over Time')
plt.xlabel('Time (s)')
plt.ylabel('Throughput (Mbps)')
plt.grid(True)

# Plot Packet Loss
plt.subplot(3, 1, 2)
time, loss = load_data('loss.dat')
plt.plot(time, loss)
plt.title('Packet Loss Rate over Time')
plt.xlabel('Time (s)')
plt.ylabel('Packet Loss Rate (packets/s)')
plt.grid(True)

# Plot Average Delay
plt.subplot(3, 1, 3)
time, delay = load_data('delay.dat')
plt.plot(time, delay * 1000)  # Convert to milliseconds
plt.title('Average Packet Delay over Time')
plt.xlabel('Time (s)')
plt.ylabel('Delay (ms)')
plt.grid(True)

plt.tight_layout()
plt.savefig('network_metrics.png')
plt.close()
