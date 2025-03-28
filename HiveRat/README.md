<details>
  <summary>Getting a connection</summary>
  <br>
  <b>Introduction:</b><br>
  As we know, the Hive builders servers are down, but let's look into the stub, in this case, 'bin.exe'.<br>
  When we open up dnSpy, we can see the stub is relatively unobfuscated despite fake protector attributes.<br><br>
  <b>Lets look at the 'Connection' method:</b><br>
  <img src="https://github.com/user-attachments/assets/d71e209f-67ed-4272-b4cd-98c22ec93184" alt="1" /><br><br>
  As we can see, a host and port is specified here, to change this, it's a simple task of using edit method.<br>
  Next up, HiveRAT includes a password for its communications, by default, this is 'Password'.<br><br>
  <b>So lets look into the stub again to see where this is:</b><br>
  <img src="https://github.com/user-attachments/assets/61d08f7a-be51-4826-a478-a22d7ded80a2" alt="2" /><br><br>
  Here, now we can see the password in the stub is 'AESPassword' which concludes the analysis now.<br><br>
  <img src="https://github.com/user-attachments/assets/5f233d0f-abf6-4fc2-bf52-7bc3d30fb5ae" alt="3" /><br><br>
  Now if we listen on port '6969' with the password set to 'AESPassword' we can allow 'bin.exe' to connect as a client.<br><br>
  Feel free to use this knowledge to analyze/research HiveRat via locahost/lan.<br>
</details>

<details>
  <summary>Screenshot</summary>
  <img src="https://raw.githubusercontent.com/Cryakl/Ultimate-RAT-Collection/refs/heads/main/HiveRat/Screenshot.png" alt="Screenshot" />
</details>
