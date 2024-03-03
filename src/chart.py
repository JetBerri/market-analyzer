import pandas as pd
import matplotlib.pyplot as plt

# Read CSV file into a pandas DataFrame
df = pd.read_csv("target/last_prices.csv", header=None, names=["Date", "Price"])

# Preprocess the 'Date' column to remove invalid values
df['Date'] = df['Date'].apply(lambda x: x if '/' in str(x) else pd.NaT)

# Convert the 'Date' column to datetime format
df['Date'] = pd.to_datetime(df['Date'], format='%d/%m/%Y', errors='coerce')

# Plot the data
plt.plot(df["Date"], df["Price"])
plt.xlabel("Date")
plt.ylabel("Price")
plt.title("Crypto Prices")
plt.xticks(rotation=45)
plt.tight_layout()
plt.show()
