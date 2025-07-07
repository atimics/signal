#!/usr/bin/env python3
"""Calculate optimal thrust-to-torque ratio for consistent acceleration feel"""

import math

# Ship parameters
mass = 400  # kg
moment_of_inertia = (1.0, 1.0, 1.0)  # Default from physics.c

# Current values
linear_force = (2000, 1500, 4000)  # N (x, y, z)
angular_torque = (300, 400, 250)  # N⋅m (pitch, yaw, roll)

# Calculate accelerations
linear_accel = [f/mass for f in linear_force]
angular_accel = [t/i for t, i in zip(angular_torque, moment_of_inertia)]

print("Current Configuration:")
print(f"Mass: {mass} kg")
print(f"Linear forces (N): X={linear_force[0]}, Y={linear_force[1]}, Z={linear_force[2]}")
print(f"Angular torques (N⋅m): Pitch={angular_torque[0]}, Yaw={angular_torque[1]}, Roll={angular_torque[2]}")
print()

print("Resulting Accelerations:")
print(f"Linear (m/s²): X={linear_accel[0]:.1f}, Y={linear_accel[1]:.1f}, Z={linear_accel[2]:.1f}")
print(f"Angular (rad/s²): Pitch={angular_accel[0]:.1f}, Yaw={angular_accel[1]:.1f}, Roll={angular_accel[2]:.1f}")
print(f"Angular (deg/s²): Pitch={math.degrees(angular_accel[0]):.0f}°, Yaw={math.degrees(angular_accel[1]):.0f}°, Roll={math.degrees(angular_accel[2]):.0f}°")
print()

# Target: Make linear acceleration feel similar to angular
# For a ship ~5m in size, 1 rad/s² feels like 5 m/s² at the edges
ship_size = 5.0  # meters
edge_linear_from_rotation = [a * ship_size/2 for a in angular_accel]

print(f"Equivalent linear acceleration at ship edge ({ship_size/2:.1f}m from center):")
print(f"From pitch: {edge_linear_from_rotation[0]:.1f} m/s²")
print(f"From yaw: {edge_linear_from_rotation[1]:.1f} m/s²")
print(f"From roll: {edge_linear_from_rotation[2]:.1f} m/s²")
print()

# Calculate recommended thrust for consistent feel
# We want linear acceleration to match the "feel" of rotation
target_linear_accel = 25.0  # m/s² (aggressive but not unrealistic)
recommended_force = target_linear_accel * mass

print("RECOMMENDATIONS for consistent feel:")
print(f"Target linear acceleration: {target_linear_accel} m/s²")
print(f"Required thrust force: {recommended_force:.0f} N")
print()

print("Suggested configuration:")
print(f"  Forward/Back thrust: {recommended_force:.0f} N")
print(f"  Lateral thrust: {recommended_force * 0.7:.0f} N  (70% of forward)")
print(f"  Vertical thrust: {recommended_force * 0.5:.0f} N  (50% of forward)")
print()

# For realistic moment of inertia (assuming ship is roughly box-shaped)
# I = (1/12) * m * (h² + d²) for each axis
height, width, depth = 3.0, 5.0, 8.0  # meters
Ix = (1/12) * mass * (height**2 + depth**2)
Iy = (1/12) * mass * (width**2 + depth**2)
Iz = (1/12) * mass * (width**2 + height**2)

print(f"Realistic moment of inertia for {width}x{height}x{depth}m ship:")
print(f"  Ix (pitch): {Ix:.0f} kg⋅m²")
print(f"  Iy (yaw): {Iy:.0f} kg⋅m²")
print(f"  Iz (roll): {Iz:.0f} kg⋅m²")