## 🪟 How to Run on **Windows 10 / 11**

To run the **GTK+ GUI version** of the project on Windows:

### ✅ 1. Install Requirements

- [WSL2 + Ubuntu](https://learn.microsoft.com/en-us/windows/wsl/install)
- [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop/)  
  (Enable **WSL2 Integration** for your Ubuntu distribution)
- [VcXsrv (X Server for Windows)](https://sourceforge.net/projects/vcxsrv/)

---

### 🖥️ 2. Start VcXsrv

When launching VcXsrv:

- Select: ✅ **Multiple windows**
- Uncheck: 🔓 **Disable access control**

---

### 💻 3. Setup Display in WSL

In your **Ubuntu (WSL) terminal**, run:

```bash
export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
xhost +local:docker
```

---

### 🐳 4. Run the ATM Project with Docker

Make sure you're inside the project folder with the `docker-compose.yml` file.

```bash
docker compose up --build
```

---

### ✅ Done!

The GTK+ app should open in a **graphical window on your Windows desktop**.

---

### ℹ️ Tip

You can automate the display export by adding it to your `.bashrc` or `.zshrc`:

```bash
export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
```