#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <pgmspace.h>

const char* FW_VERSION = "v0.7.0 Sensor Ready";
const char* RELEASE_DATE = "29/05/2026";

// ========= ESTIMATIVA DE CONSUMO =========
const float AMP_POWER_W = 500.0;
const float BASELINE_HOURS_YEAR = 24.0 * 365.0;

// ======================================================
// SAPA IoT v0.6.0 - Dashboard Industrial
// Base funcional preservada da v0.4.2:
// - Pinos reais: SSR 23, Botao 22, LEDs 19/18/5
// - ESP32 cria sua propria rede Wi-Fi: SAPA_IOT / sapa1234
// - IP local do ESP32 em modo AP: 192.168.4.1
// - Browser Time Sync
// - Teste 60/120s
// - Agenda semanal com ate 8 janelas por dia
// ======================================================

// ========= WIFI STANDALONE AP =========
// O ESP32 cria a propria rede Wi-Fi.
// Conecte o celular/notebook nessa rede e acesse: http://192.168.4.1
const char* ap_ssid = "SAPA_IOT";
const char* ap_password = "sapa1234";

IPAddress ap_IP(192, 168, 4, 1);
IPAddress ap_gateway(192, 168, 4, 1);
IPAddress ap_subnet(255, 255, 255, 0);

// ========= LOGO SAPA IoT EMBUTIDO EM BASE64 =========
// Logo otimizado em WebP para evitar upload LittleFS separado.
const char SAPA_LOGO[] PROGMEM =
  "data:image/webp;base64,UklGRjoeAABXRUJQVlA4IC4eAABQfgCdASpoAXIAPpFEmkqlo6Ihp5U6uLASCWc8VqDfvdIN/jx2W"
  "BP4dEToAf//0+cgP//zZP4L9Kf9D2G/3/ox//UqFuK37uCzXIVuGhpLbX+39YnK3evXnr/Och9v5F8H2Klxf5nwF+5OdfsP/of4b"
  "mVXpnzygD63ebX9Z+wHqP/f/5f2If/H6dd9cdR/j////p/Wx+x+ruVSu8JfqqNtj8+XljNsgkNxl2A/e2PeMd0IxyHOPM2kut84c"
  "+2uXJUMUSRXIN/rO6Hhh4YK7EjYg/QEYitS11cy4WFZCER985aDV+W8s41ORZKUpostSevrRYsvBsj7IJ/e+ebBaUSdrXcuB3VH+"
  "Nk9PEukWK7YGsNFZR0E0PLK+eBnZsYSwHWZW+OsqSFt6HMe6dEsP+u5DspP19CDgQNDiRZQqBmUfBsMHXenQ/EnZoU93E3zO5THB"
  "4zgTOSzfyzD+oi5N7kiVyN664FE9/HPr8QPAfuKEciTL+Fz5rz4VUKgaRhypWra8TdgFYqRoDKTCOWLdm0IEYMe1qS3DxM8LD87L"
  "Ue/jNSTQjNNB/K0V05dNhVBJUvdPZpSxS+XMVp3FjT54MR1+Yi02fkXqzhA6DJgZQxOleGMRmFtr42MEpjpHWbPh9uEkVZdRDUkT"
  "l+CMctoj5zCVG5IomiH7Bu96d6qTbp2N326AcsfTsgp9QjWhmN4JHFve1Gzp6N1bTAXzbxBY6SpLi8RXNIqCZb7CSCeixFYmUQSw"
  "7HmbbxM4g+jNWwShrXNLsuTsoOjrXn4RHZTxiJxWoE7q8IaPN4hvafFnQc1fF6fMxuCvwLCYgefz9PS4Ydcv87uQZmf5Om0tUYQi"
  "Gh0ZdjTdpK66nxRNVUTBkSIXt9O9M3Ffa5Xj4wu8a5UEBb93ITM1jOAQ513JAKLWJSjrTgl7ulwMogf+hK9kfrye1sW0gbd3ccoW"
  "HKg8oSAoPbCA03pX20Muiv3DbSia3sYY5Sw4rMLRMa/CZNX85m7Yi0Gx9qJ0VxoHobLwfQeIlE7HBuj1fSkb9qwAbc3vhglyMjSl"
  "DFxY0F/kBuNWAj1y1C3k2g2bbqOItm8sZTULRd/7mucGoOlNMr1Fx6E3unF+BJ0YlsOdLjRe4rKdK02/giAdIUOSV1HZvUUcaXJM"
  "2EXsOHCxit3hiPdUNdfFJ7yf3lIb22apWaeNiIyR4fQBzyb0UyfV7yrUo5mfHs2fmzb1nqTTcoy+QSFKzB2lRfz80QacukuTIUt/"
  "4y93SzBqNtvHyR5Ql7G+055wc7vWGHxk2YXqFL1UXiGA5BLHHwSq1AywR3xIzLsjxAwosuf1msmuqxTGfNsfHyagCyIaeoZ3wme5"
  "SiHES5UAAD++IvzDBVlMNn0RB3bNiwWLDRt4XR1wG+yxUzZsTRIzu89RsTt072vUFIZuRBETHmVnyt32Zsoc0SWMj+3zTPe70Nrb"
  "QW0CjoKXp70rqntNlCZ4kWNiq/1K35YzYYfV/xLbkF4D9cih+4LcXgSkKIcv9MpKNxfSPHrRLL7129/f6yssC2mKu2JAzOPpVP5R"
  "cOk1Mz4TOhzLNDOJzljuNaJ43vRjhueb77yhSfD5XA9xLtz2rhGw1vFsHSMLbp+D/f1znok19PYmnqQztNN5WOwdtuE9tXgjP177"
  "6GHh+Hg/yjml7+khaXDGZOy5PfiJUD4ZcfIv23z4fZ6qFvE3zWKrxmQTEhF+5KvTW4aJwyeDqVknnAa+EK/mBCY+7+5JcWjGaUpf"
  "dR1Z39j88kR/PK2MX9Lk1WdtshOq0LRzNkCdX1DKqUshiLN+kM6OmHE0Rjb51vX8GbrT7WIQzeUlxokfRta2UfV+VglVvg1StitH"
  "6umJ8zWAcw0ee0DycQMcHcssIdPPwgXZ938j63IzVbwMxEl77E2IQf0yAiy2vfqjqNLgcOkLwfnXi5ZdRAwVfzIsXMHDdUHQHlG3"
  "ouwYXlfNnRuuXU8T64+dirAHvLuO2ltAaz0t457x/dIaeW8IBOIj5Fyao/isreJaWG6JFJwGXcUahIypGxfeiBUbD4eq7icYA5p1"
  "1AQTnV6xF8dr2GNY50O62ARaLeNxE2Yk6qb0MZH/owpxFYiepcuMxYO3aTghd2r7gAJzhp5WsTZHWZOGJgO44a6S9G0QgmMuMf4U"
  "5gZ2KN1wV2Fwdlu3i5LP4+2KAWyDOoqXSTrtqcB772v7g414pvQxhsOvM+m2UKul3OVwdiK+wUivXnbrT7ZcTHVaBwfDVSWsHVsv"
  "PtuDfBzIlxg1G+6+Q7KjHwi+6nJ6PJEZtLL/A06PFMRb0Ah1OGH9X2ARwKgpuoB9LkTy/xFE88CZh4yjFmGK16QFFVgkyN78KssE"
  "touX9aVUrouv8jet51f+SOstYeACJLu0rAHWfMC24UQM9fWD4D/SSa5UIlhmDfgxwuM/ksF5Awl5Cdg640/KOaFR73ydD066hYz3"
  "8PVdz5WGQevp7Elibcm0ThjjawXsznsqzWzftO0hG1BYf+CvD6O6g/hYkiPKyRQl9MbPWJZPd2+4VSJs8fr5AF8O8a64SU/BVrDJ"
  "w/9zRLhTACWeTWWHSV9Ag0mfPag5u7eB0inqkt5I4q9cYJ5MYmAT4UKHapnMqTUqDP8w1LFVHX/bMqvYqPr1rtb0kTZMf7feWftH"
  "osQJJRksHaiY9D/ZbCPHP9DcqkP5/sdQ/fcFxrmtxJ9HkL3B8fiIFbkR7aPI3v8SocA7RK/gj/V7iDgczNJ0z8bs0eZ8Z18KY3aO"
  "wznvj40sd94+8q2+fZIf8uFpQ6EsvazbACMLW2HfZRFZ5HMdP97Pzy59cOJP2V8c33/nmj0HrdILFGdWW7mrvXGX0bztG5iInSmt"
  "ymIIzBvO/PJtfPxW+FKlPTj55ie332bHTNt/k3j2jZZTuhutWrNBilomoaF8fwp71UZG1qNfleqf4W2KE2owSfqVpnr0BZwhf6qv"
  "ZnCz/SdPtjdwx1axjhzlhnyzxkv2Atq050wNi0tZOt/w4DrtNvxYPi5eAFq1OhK0eUx+cPadSZ7NjGGLfLi/j2E8iiAUx0Rp95Mh"
  "2VJj1GVjIbao4d7dkQZleX8GK+MN5tHfs7qMKxvnbWAuPG2l1BxC98K88gcMoCGGweGJbok17AuCP2OVWlA3pkVLBSKDQPtAsakB"
  "aLP68ccUgBG9sbuGOkkicvNtJIvPn0fHvnxlQ+85wIxLGBf/3APQuAzX5eqExb2hPyoIXbVp0J9Ru4ihj9p+oARuweaPj9CBOpb4"
  "dhPyovs1smlQ38osPaRAxasr5OpF5X4WqMnCrE1RJ2yFFTMfPLlGuJGCXzTjRbhIaqyMhy0LnnkXw1GmpLpJsaPtpnzxsvxaUM6J"
  "yItCbAkrReBsOUWNKPXxONB7777CpcKfOdvc5XeI16g5/14436PNcQ/eWMo/csfBr8FBWGpZpMrOiHoyx3Jd4WmvEP4U96qZof36"
  "HeBwxArwDpxlJy2F6iYweYVrjpJ/wPCl2jX+JfSRs9ziI+VuIZxXYP+VcDF/jD4EOXKDIWBxZYAqi+JOsttQhFot43xIHCENCTN6"
  "JxCZvMQS5cOScrg7OEbd1yitbpDhISiKaMR3FZ8yP2AjrHOLV9DuZdjbzV0+LGC7F6sOcGjp+ayJK83fCkwdoRBivLu9Ir4N/6Fw"
  "O4hRry3e0bxm5YIXBuPtEgDo5zzM16qhc02xiYdnk0V/A50pax1y/e1i2qCP6W7MMDvdFgPrBOloS/8zXCq2FJJrTf6jYMmnL2qu"
  "W5ui2wwO+FSptnj68KrL+tVRbBE8GvhqtYt95MlvWDFWbrtC99HGcrUdphEcEidebfBNq7C5f+WoXggc1FGxxuO5qU3rA+qlZaA0"
  "Wvj1kBYrwUrJvIR6ojbQr3lh/Xd1SJI77IdRR/itKXLFE/7bOAtqPtQc8RtASGHO4ntGqLRDq7S1tRdPh4y4Hbe7Tamf48O5Sm32"
  "FWoCjkn7Rz/DJLmQad3Pon7ZIzRWrPGBSfSEkxpfuL8bD+x0meGfMI708EWejUW/DHGdQPrDzyaGiGD0xEmnlOTVux74U1Cc5OJW"
  "myZ+NmLj8MGvH+G+QLkRL8dgpydKq+xn422LGH41+8sAaBot9K+eGEvEOMcL3qYhitDdJo8jhViu8+PRKL9Todm9OMCeqG7NC921"
  "77rWtZtJgKZWc2P1q/IEcRNRbNL0onbCMaNN/SMZ64gz/hhqkN8C8iM1DayEA3BkxnmfXS+Rb0X+VP5kDMekfH118+/FlTa0nnPW"
  "morO6y95mgYUohb4qzgC+QiAwuEXamTtuFvKMprkPpKUxw2KlFjxLM1zNo6DosEw7KjeHfPBwALYULj273ETXmE+Olc2/GNog7AQ"
  "k/uip73gnHuMMxiTphNb2xZI2ptNWa9dJKVthAdtEocm5WQiy3CMjThB8ZQjnlJvGW9EhKhx8b4DS2mUMKHwx3Pdd3/rb+wpXBc5"
  "ugiGuVP88Ke0L5W0RdIy4x/hTmBkX5iw+nL9Rhd7Oh1UqSpr47FfmREOwT/ogW+hRcXl3dGLB2dE2zbVwqptUjVCmvba2/+GUiIZ"
  "G1QCQtQnAquuiOAA0HLZ0AC0rGN5czA1K0aU2NMqtXgiTwnOMf+x8FwfbTxj9t+i/a79SRb/3yUxkImGMUi9o0mB5um1BF+1kWM3"
  "9K6GHiA5VTUDMWJ09XBMlikpFFMg1MLUJ5QZJWzHmc6tsGNQBXPpL3hce8QC2/1Nz9JeMi+eZJlqg6FFpDiIXfvfbwU0sRk2KAPk"
  "9DLonduqAUr5jW9GammDk8qc2QEGAvm2OnLt8x+wxx9jOqA3s6gu60olODVzcaNcl5Juq1XB1nSPyr/OEbTDjGeUZvM/gsVdxjg2"
  "tChnHbvnpUBW4T65qGdK5DpBnTWowSmNTQwVL3oDhSwGQ7TZKgtA5S8I285cHuDu9kJRWNkrWFfW/11D0K0/H0G5pRI+poeaWfo5"
  "eCzRuoBP54rrpuiiphnzlDiUg8ZOz/9kNNfrLQzo+YZLCgl1fB0N0VdXi7bb5K652iMQYYEquLpp/J4l+alSTNiQ5B0b2JmWpyP7"
  "Q7Q1/nqriRPC3qpv8rP0fvvoTKsM5DtZ6Lw7ESyC1TTwOPTn4RiwskpeLhQGTMT491HDc5KU3fv3ByfMQAxwduBx50ZG2herXa27"
  "frbBC6vOVcCx5DSIFtPS2ThbAZIaW1HgSwB9fnL2UsWiu2NsPv2jKPWQ6+DXP6vPhjgnn/TaypKw/n97yPTM0ux/7l0u0fba9bWu"
  "2QDUFLqOC5LH9inereFiyr20yLaZhWUE/UFS4oN/KH4+CvlPGkcWcqRPjocyChAtmWKbkjdyWNsEkIHBQSwaco82XaVZ4REQ2fHs"
  "gcesvmr71mjgegnGoyeK+3UpKAD4iJnL35fRJQhrSVs5cUdCK0gdBTpobkFyl01PL+XM5+Wy8G2IwLEcY4YnMALJWIPrDwuUVfz5"
  "GGEaUikHThNS6cnDkI3UDi/e8Z0Pe0mXuyXKjSbuN++ZD0o4bdV1blVxwgbR3rf5HL48WNmwHUyeqDfC+yQewD4Q/m7FYzQqozzc"
  "M0OtQO+MVgWA7HoRsHbkCW2fzLXw6XR4ORsDH/ireaO9GJUEVK19kFVXxnQorfannEoOzX8C77phvgcE1BmGXhR/YRmYbgBBDVdT"
  "VJAApDwFL5/gelGti4NLaCw8VxMtUtEHZU4qmbFeyu06q+nnysm3e3Pl7rpmVBOQBMnOmKDnARQv6lStsdQ6z9AY60k7IZno/g3E"
  "ciO24X6xF6twGVuAgq9vLmkELbu6sOzZ4wuW3yb4wulnBRN/Bn268hQvhLecihHVuNcja4T5DS5Bk6Rl7IH8u8geb7B5Uslfram0"
  "gZq9kgVXd3Fr22Sp7ualiGko54FPmtA0O2EIRL275j311cDf3NT+7oHz/IkeOtRQ9PWlKeV9+igVng2reslxbIDkGCGGX0v+OQ9V"
  "uV8tr3j6fZt+ac6phJSll5ENIB4FM8H3rRZpmvp1CNTTNqXzWLttNRekllJVX3Y0PufRi0chEWziyWgiTjyOImXk+eq6gnmSMad9"
  "a0ANK1IGXVRY3fAC6wn4bzWjwt4hLbxG1/KMdgHKCqFJWtJDYm4tf32Ar2T7wx8WLvqHi6ujjDunJU7bHAmNZz9wd10I6YhqqVRA"
  "Iv1O8KjrY0nG+d3Lz5RU6Waa8p2btjxi8MdZZaAbJMbova3+0STAheTo2btPpHrd3ZOkhPhe+a7BUbp1c6EDD+BK3fKYv7QvuWlS"
  "JBsdIBjaEzWUf97sRsulLxZ6etxp0ErW0N3CbI9Y3wkyT0PFaReN3SNLtCcx+bQoOoRqDzI0KdiIhm/jPDTTSkmXgEOe5GjElXcQ"
  "FW6TSu4Jl5DW9iZGdQlvmcxzMRF8sfwT8E+iaN24DKCAeFGf+Wljf3Jz38GpC7pz4C4mKmy0q0ipD/VKyoclWBIIWv/EbFgZYrWj"
  "Rb44B8uYF7ucT6E9LFR5I2aeoST3PCRiNQFG95WpxycuYa044KNh1mSpOwZZcZqCSt/2pSFK+x9lS14bNE0Vf0cktx4sMtzw9AEl"
  "LK/pVW50Tz1U0MqCdMbzJog8v4Ps9z210EGp+gzZJOpqx8ALgt1k1U2yuTbpj/EP2Pv+N/x3RAxuT4sKMSn6cztUVeHtGImPVivZ"
  "kUrj5MaU8MC8rOs/D17TV5BFotE6lxlntiW91aVuf/7pK+xhxyhUJJvHmxCMEh+9Tma3ew2wWf0x1G2f9ZaFlKCzUi1yw8llT4ys"
  "Khm+6eIoyzi0tcZOLU5s9MnSEahtJEjNFoWMJMfFi73khfhHhc8d8e1mZifxCywiWCDJUkzVXm/wyq+l15xD/klM8R5dnfNFFBNE"
  "oiZRhdk1tmKcKiDgcuwU4TOxxFCUTq2Y9OPqdsHNQRwTJAs30PItmO2xwTx7Gjxwu44lOsWr2YlyzKxurCEwcy2RIVpPZJ28pGFf"
  "6MtsdOIqJqUymJ0Vx0XtbMRqsgjucvEKnZPXduj60jOafZbL6++MdXb02uTi9bLVOsbqmWBcio6ur3oIFxX9VV0j8PQKbaSwCoc6"
  "6sdyxakYZjOMTbqLBn8ex9BlpKPUwJtRDmLo/BpCf9B6RBVCv241YJx6fHRlP9zHfp5rthHZabHDXSVFqEQF/ebOekfNgMXPvs8W"
  "/mTIglf1lsXW+ZTet51f+c4rAL/NMN0k0V90lKs/9s5zmpVdGT7j+aBOksklkbDo9YRcxsEwJoLVFCKCPlNOKq5CVzEo4VfB1zVQ"
  "vsGyAQKK97VPBJF2yIBTA0Fj4TVtNCiG7dr9IINF5LTIwzUDFmolv5Q6blJ5wklDXA6ALqv5Gztzps4phKwgTQDHMS3+/3DQhizC"
  "84CABkLzcsrcQURDbfEmzAY0C1Wqw6IA8DbYY5letGJoCZlTmRizPSLssIwSHc7KZci+3G6SyTqMBXnuOfGa/5yE56TiyxWIixDB"
  "buMvl0SAndDd/VuzXBM9LYs0vH4dV6KfcVd8ufaof+M6WmNRbMIrjxQ8gTXEVm7JePeMGYgbHw+aKbHprocAwsDU0is7OwrpM9bO"
  "l9xZ1v75aW6UayxH40j6PkOPqO3OdJYB92w9pzm0D5pI5NauOcn9xzuKInuwrYtrn6bKfT5WIWDs6WMYyt6Tpm4ETIvh/+k+HTtG"
  "U399BWtazlb5JFTSKSCbo72pP75Uvs8/tMNYL7hb7c7l0Z7BtQthPW/bCLTk3fzMDy1XoJI0UqqO/yvyYRqZ5faAVUzpwHdoZSW3"
  "MhSHbCUAtf2p0fpuFOEJliQstoxF/Rh99Yk01b25gbvH84alkFU7plPGwGo/QsdmsF3HcIZ2KiFv+xxv6h5X7YJjd07kYd9qCNjK"
  "vSLX7PP6Z8inXlwpXhJPjB1l2Q8NbeJsFa/kxIO7P97+fYjVVUGMjjosqmtx/NzJNxe8SM5Isc6GQJLlE3gX0AWgNGW8pW+aT0Ln"
  "ieQvdfOML4/hY3APincLduOr9MrPtMYTF6ZlNEq4RIodHaQL1ZVAmsGSzDPBAsJ9VuvCf3lSS/pfQEpNlKqwKRopW94VmIYUhoav"
  "jIJqeFHZs6SiThNLZHG81vkRzYfXPkvvm3tWoSqFgJj1MZvfoLNLa8Pz7+CpUhhZxcKpUfT4fE+Ox15W+Sok+6bnDgxbBQ2GUO9J"
  "gMNXywrdyuc08wuFPJ/4cfDlJFYKvEWw4IX3kd7UgCNDK0BjjnE61aU2Haoz8j5qnDjaqqDVUeolDTDvI4C31357WFIhLlpIrzr7"
  "/8DWGCg2uQQ4+3m4t01bZjb+Uo/h6Xat2H6gvIIMKRmz8K5RQ78C6YlDqxmdlO0/fRpgAtf2sL1zyVWPBmjO+nehRVTY2sL59DBU"
  "WXSWSErnK5NvthtW7OpebTIu3vLtF+wAq1Pd2CB9kTIAlUGcAWUi4ma9UyGR29j9YKRLtsuKINdeTSLVoGWtxrr1P6YZ1Dtcp7os"
  "heqDyUaSS+YK5M2fjpW2GQ3jpN1hkx3qbofd9Xrtaphq86rVrI35BKLGUCA0FEAHBg86knQYUai2BTI5ae4PnPM2BdfEjfSjNlT5"
  "uJqg3tMb5AeWtj/NOBAjGosut6pUCrH/UYumil+zsz83nz7nEFu5WPbOZd9/0ZgeHPq7SP/qZpDItprCWJlVUE/EHjuOkrZ0P5dd"
  "v2b3S2/w+5Qcs8csvoSOzFBqUE4ASoXf/SnvsS+mEf46/Ta2+AqdurKpzFwCqGhVis28ZU7NnDtJd2JSEA6fTEmk5y4vMqBWrlqg"
  "CZJNHFf8yYyCJhu9s+6MwtG+cwTs/jOvrw5vgFn7qy+UP2p8gWToY1sB8iA2hz8RqvROzHlze7lyJKMFvRNYSYPw8zhbc/mMN+TI"
  "hrz9grEwyuwNMt+TFbnYbTVa54dttmCdyi+oJzPWFEMNLV/AoRWDX8nGMM5qG75Hnt+KilGI2H8DMifzYWVTbhaVgxk9AfaZ5Yzq"
  "HEFPCf+4iotP4NMnsT+1XBRCv0NYDbPJwwgYOicDG14vp1bHJRPQg5TjJ9aKa+92mXoDkqNYBHk/QRR0CVdMMvpvZ0RMlygqamyn"
  "xB7lloJg3u69V8n2pbTQOBydkfS8nvUeUR4JQ8kUijVgigzyTWLN8xr+AttZlVSfQuCfu8FaBs5tMMWjAsbZQUIZ7pPsmXkqfN9S"
  "PSQEehMMWJIgMfrbIA/vrribxso2HFtEPTS/UhJo6pRJ8XFHw948Ukr6KWhYCCraVHXqpkmErzTooqdcqB90LksFql9CeYoo9+Lm"
  "iViJPBJU1gvWOCOxDMNmz6nQM6WJLJy9yelB3zjbJflSGKPtxXvZH/Wn6KR/XH5rqTGKcM2mGBKjVQ5DPL/xEkDNT8yLLVWDE456"
  "L6KALVqOC03Hw1k7OO2buW8H2fpZitKv6p+BQD0RIOb+w3A8aAyDL0eF5rsFzlATzfbT6kFbm8qgCjrmakJWQxnCA7sT61ei5jeI"
  "912+wAak+Rz+TXW4FUGvrELhpcZdVMM6zalkNaUOuOVtIR+SantMBB8lnmGA+mOz1FSgp/mzb/GN2VVbTCqg88IvYO01nTrQs2ss"
  "lItYm2wTXwcpmnD25Vvoaa4tkOqHqaW7ybIK3JqE61jhQeW3BNQMz6CE2sZuSyRnAasEUGeZEnSnbg5r9WLL+XhCDcob7h/LuK2u"
  "SObS/Eg80ZQ5pHnAb8ljFdDWmzGKBtWRzNVV9S7N3L1qtR7keDlmqkTabjnCzPe2JikUf1s13cm9dXOMsMU1d/FXG9BBD/g2eycM"
  "iy7zFDKk8MM8ryEPhqGfjAe6Ewr5hYsIRAe/aTmz9DccGBTQkqyfRWpP+bK7bClevC2s0UXBCvbY6Hc/UPP5Rl2RpkdvWRGuy1Gl"
  "YZO5ADOCpDtcNi+7GeB8Ns9gb5l2e3nIpcFmEGMSyWNhIys5KAaLyZAut07dcjnY5m4dWWiJ11fIgIfHONpuFy6MQ3xJCKsO5ttN"
  "9itOvquaI/fiMHntGyYsiLlMr4PIgVyuFXLB7tx1oYG287iMYCiXlRM8qJ50h5mxf1NYYQ8bYFpQH2lM1j8M6L8y5C2LGbtx5VWj"
  "6f9CQ+JpQPs1a1molAVLw+q8MS9YWVciVetkRKU7l0Jgxr4kRcQ7RIoTlp3uJeubD+PTi9319gG0Ym1coGwK4PAMwoYMsRgiJ85U"
  "5/oMNSK0/buTJ1BU6/dWsEZ81rGCkVAlnDkcQaVfsW7QrPvv2gHAqaUB7VygX2wpRKQYzTnPL4wls+Y+qGF4NEpHUcM5WthDozeQ"
  "WJfaAElm5Dy4869EDBw6DRDdptw3SVYr0i8nvAYAQORLNW5AAAA";

// ========= PINOS REAIS DA v0.3.1 =========
#define SSR_PIN 23
#define BTN_PIN 22
#define LED_GREEN 19
#define LED_RED 18
#define LED_YELLOW 5

#define MAX_WINDOWS_PER_DAY 8

WebServer server(80);

// ========= LOG CSV PERSISTENTE (LittleFS) =========
const char* LOG_FILE = "/events.csv";
const size_t MAX_LOG_FILE_BYTES = 1024 * 1024;  // limite de 1 MB
bool littleFsReady = false;

void ensureCsvHeader();
void appendCsvLog(String msg);
void checkLogRotation();

// ========= LOG INTERNO CIRCULAR =========
#define MAX_LOGS 60
String eventLogs[MAX_LOGS];
int logIndex = 0;
int logCount = 0;

// ========= ESTATISTICAS DO DASHBOARD =========
String lastEventDashboard = "Sistema iniciado";
String lastCommandSource = "BOOT";

unsigned long totalOnEvents = 0;
unsigned long totalOffEvents = 0;

// ========= SENSOR READY (PLACEHOLDERS) =========
// Valores reservados para futura integração de sensores reais.
// -999.0 indica sensor ainda nao instalado.
float tempAmbientC = -999.0;
float tempInternalC = -999.0;

String getUptimeRawForLog() {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;
  String hh = hours < 10 ? "0" + String(hours) : String(hours);
  String mm = minutes < 10 ? "0" + String(minutes) : String(minutes);
  String ss = seconds < 10 ? "0" + String(seconds) : String(seconds);
  return hh + ":" + mm + ":" + ss;
}

void addLog(String msg) {
  String line = "[UP " + getUptimeRawForLog() + "] " + msg;
  eventLogs[logIndex] = line;
  logIndex = (logIndex + 1) % MAX_LOGS;
  if (logCount < MAX_LOGS) logCount++;
  Serial.println(line);

  // Tambem grava em CSV persistente, quando LittleFS estiver ativo.
  appendCsvLog(msg);
}

void sanitizeCsvField(String& value) {
  value.replace("\r", " ");
  value.replace("\n", " ");
  value.replace(",", ";");
}

void ensureCsvHeader() {
  if (!littleFsReady) return;

  if (!LittleFS.exists(LOG_FILE)) {
    File f = LittleFS.open(LOG_FILE, FILE_WRITE);
    if (f) {
      f.println("uptime,event");
      f.close();
    }
  }
}

void checkLogRotation() {
  if (!littleFsReady) return;
  if (!LittleFS.exists(LOG_FILE)) return;

  File f = LittleFS.open(LOG_FILE, FILE_READ);
  if (!f) return;
  size_t sz = f.size();
  f.close();

  if (sz >= MAX_LOG_FILE_BYTES) {
    LittleFS.remove("/events_old.csv");
    LittleFS.rename(LOG_FILE, "/events_old.csv");
    ensureCsvHeader();
  }
}

void appendCsvLog(String msg) {
  if (!littleFsReady) return;

  checkLogRotation();
  ensureCsvHeader();

  sanitizeCsvField(msg);

  File f = LittleFS.open(LOG_FILE, FILE_APPEND);
  if (!f) return;

  f.print(getUptimeRawForLog());
  f.print(",");
  f.println(msg);
  f.close();
}


// ========= ESTADOS =========
bool outputState = false;
bool autoMode = false;
bool testMode = false;
bool browserTimeOk = false;

// ========= BOTAO =========
unsigned long lastButtonTime = 0;
const unsigned long debounceDelay = 300;

// ========= HORA VIA NAVEGADOR =========
unsigned long browserBaseMillis = 0;
int browserWday = 1;
int browserHour = 0;
int browserMinute = 0;
int browserSecond = 0;

unsigned long lastScheduleCheck = 0;

// ========= AGENDA MULTI-JANELA =========
// 0=Domingo, 1=Segunda, 2=Terca, 3=Quarta, 4=Quinta, 5=Sexta, 6=Sabado

struct TimeWindow {
  int onHour;
  int onMin;
  int offHour;
  int offMin;
  bool enabled;
};

TimeWindow scheduleTable[7][MAX_WINDOWS_PER_DAY];

String wdayName(int wday) {
  String names[7] = { "Domingo", "Segunda", "Terca", "Quarta", "Quinta", "Sexta", "Sabado" };
  if (wday < 0 || wday > 6) return "Dia invalido";
  return names[wday];
}

String twoDigits(int v) {
  return v < 10 ? "0" + String(v) : String(v);
}

int toMinutes(int h, int m) {
  return h * 60 + m;
}

void clearSchedule() {
  for (int d = 0; d < 7; d++) {
    for (int w = 0; w < MAX_WINDOWS_PER_DAY; w++) {
      scheduleTable[d][w].onHour = 0;
      scheduleTable[d][w].onMin = 0;
      scheduleTable[d][w].offHour = 0;
      scheduleTable[d][w].offMin = 0;
      scheduleTable[d][w].enabled = false;
    }
  }
}

void setWindow(int day, int idx, int onH, int onM, int offH, int offM, bool en) {
  if (day < 0 || day > 6 || idx < 0 || idx >= MAX_WINDOWS_PER_DAY) return;

  scheduleTable[day][idx].onHour = constrain(onH, 0, 23);
  scheduleTable[day][idx].onMin = constrain(onM, 0, 59);
  scheduleTable[day][idx].offHour = constrain(offH, 0, 23);
  scheduleTable[day][idx].offMin = constrain(offM, 0, 59);
  scheduleTable[day][idx].enabled = en;
}

void loadDefaultMultiWindowSchedule() {
  clearSchedule();

  // Segunda a Sabado - janelas curtas para avisos da Alexa/amplificador
  for (int d = 1; d <= 6; d++) {
    setWindow(d, 0, 5, 58, 6, 10, true);    // fim 3o turno / inicio 1o
    setWindow(d, 1, 6, 58, 7, 10, true);    // ESD/calcanheira
    setWindow(d, 2, 8, 58, 9, 10, true);    // estacionamento
    setWindow(d, 3, 12, 58, 13, 10, true);  // ESD/calcanheira
    setWindow(d, 4, 13, 58, 14, 10, true);  // fim 1o turno / inicio 2o
    setWindow(d, 5, 21, 58, 22, 10, true);  // fim 2o turno
  }

  // Domingo - preparacao para 3o turno
  setWindow(0, 0, 21, 45, 22, 10, true);
}

// ========= SAIDA FISICA =========

void applyOutput() {
  // IMPORTANTE: logica preservada da v0.3.1 funcional
  digitalWrite(SSR_PIN, outputState ? HIGH : LOW);
  digitalWrite(LED_GREEN, outputState ? HIGH : LOW);
  digitalWrite(LED_RED, outputState ? LOW : HIGH);
  digitalWrite(LED_YELLOW, HIGH);
}

void setOutput(bool state, String source) {
  if (outputState == state) return;

  outputState = state;
  applyOutput();

  lastEventDashboard = outputState ? "SSR ON" : "SSR OFF";
  lastCommandSource = source;

  if (outputState) {
    totalOnEvents++;
  } else {
    totalOffEvents++;
  }

  addLog("[OUTPUT] " + String(outputState ? "ON" : "OFF") + " | Fonte: " + source);
}

// ========= TEMPO =========


// ========= CALCULO DE ECONOMIA ESTIMADA =========
int getWindowDurationMinutesByValues(bool enabled, int onHour, int onMin, int offHour, int offMin) {
  if (!enabled) return 0;
  int startMin = onHour * 60 + onMin;
  int endMin = offHour * 60 + offMin;
  if (startMin <= endMin) return endMin - startMin;
  return (1440 - startMin) + endMin;
}

float getScheduledOnHoursWeek() {
  int totalMinutes = 0;
  for (int d = 0; d < 7; d++) {
    for (int w = 0; w < MAX_WINDOWS_PER_DAY; w++) {
      totalMinutes += getWindowDurationMinutesByValues(scheduleTable[d][w].enabled, scheduleTable[d][w].onHour, scheduleTable[d][w].onMin, scheduleTable[d][w].offHour, scheduleTable[d][w].offMin);
    }
  }
  return totalMinutes / 60.0;
}

float getScheduledOnHoursYear() { return getScheduledOnHoursWeek() * 52.1429; }
float getBaselineKwhYear() { return (AMP_POWER_W * BASELINE_HOURS_YEAR) / 1000.0; }
float getSapaKwhYear() { return (AMP_POWER_W * getScheduledOnHoursYear()) / 1000.0; }
float getEstimatedSavedKwhYear() { float s=getBaselineKwhYear()-getSapaKwhYear(); return s<0?0:s; }
float getEstimatedSavedHoursYear() { float s=BASELINE_HOURS_YEAR-getScheduledOnHoursYear(); return s<0?0:s; }
String formatFloat1(float value){ return String(value,1); }

String getTemperatureDisplay(float value) {
  if (value < -100.0) return "N/I";
  return String(value, 1) + " &deg;C";
}

String getThermalStatus() {
  if (tempAmbientC < -100.0 && tempInternalC < -100.0) return "Sensores N/I";
  float maxTemp = tempAmbientC;
  if (tempInternalC > maxTemp) maxTemp = tempInternalC;
  if (maxTemp >= 45.0) return "CRITICO";
  if (maxTemp >= 35.0) return "ALERTA";
  return "NORMAL";
}

String getUptimeString() {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;

  return twoDigits(hours) + ":" + twoDigits(minutes) + ":" + twoDigits(seconds);
}

void getBrowserClock(int& wday, int& hour, int& minute, int& second) {
  unsigned long elapsed = (millis() - browserBaseMillis) / 1000;

  long totalSeconds = browserSecond + elapsed;
  second = totalSeconds % 60;

  long totalMinutes = browserMinute + (totalSeconds / 60);
  minute = totalMinutes % 60;

  long totalHours = browserHour + (totalMinutes / 60);
  hour = totalHours % 24;

  int daysPassed = totalHours / 24;
  wday = (browserWday + daysPassed) % 7;
}

String getTimeString() {
  if (!browserTimeOk) return "Aguardando sincronizacao";

  int wday, h, m, s;
  getBrowserClock(wday, h, m, s);

  return wdayName(wday) + " " + twoDigits(h) + ":" + twoDigits(m) + ":" + twoDigits(s);
}


// ========= AGENDA MULTI-JANELA =========

bool isWindowActiveNow(int day, int nowMin) {
  for (int w = 0; w < MAX_WINDOWS_PER_DAY; w++) {
    TimeWindow tw = scheduleTable[day][w];
    if (!tw.enabled) continue;

    int startMin = toMinutes(tw.onHour, tw.onMin);
    int endMin = toMinutes(tw.offHour, tw.offMin);

    // Janela normal no mesmo dia
    if (startMin <= endMin) {
      if (nowMin >= startMin && nowMin < endMin) return true;
    }
    // Janela atravessando meia-noite
    else {
      if (nowMin >= startMin || nowMin < endMin) return true;
    }
  }

  return false;
}

bool isIndustrialWindowNow() {
  if (!browserTimeOk) return false;

  int wday, h, m, s;
  getBrowserClock(wday, h, m, s);

  int nowMin = toMinutes(h, m);

  if (isWindowActiveNow(wday, nowMin)) return true;

  // Garante suporte a janela do dia anterior atravessando meia-noite
  int prevDay = (wday + 6) % 7;
  for (int w = 0; w < MAX_WINDOWS_PER_DAY; w++) {
    TimeWindow tw = scheduleTable[prevDay][w];
    if (!tw.enabled) continue;

    int startMin = toMinutes(tw.onHour, tw.onMin);
    int endMin = toMinutes(tw.offHour, tw.offMin);

    if (startMin > endMin && nowMin < endMin) return true;
  }

  return false;
}

String getNextIndustrialEvent() {
  if (!browserTimeOk) return "Sincronize com o navegador";

  int wday, h, m, s;
  getBrowserClock(wday, h, m, s);

  int nowAbs = wday * 1440 + toMinutes(h, m);
  int bestDiff = 999999;
  String bestEvent = "Nenhum evento programado";

  for (int dayOffset = 0; dayOffset < 7; dayOffset++) {
    int d = (wday + dayOffset) % 7;

    for (int w = 0; w < MAX_WINDOWS_PER_DAY; w++) {
      TimeWindow tw = scheduleTable[d][w];
      if (!tw.enabled) continue;

      int startDayAbs = (wday + dayOffset) * 1440 + toMinutes(tw.onHour, tw.onMin);
      int endDayAbs = (wday + dayOffset) * 1440 + toMinutes(tw.offHour, tw.offMin);

      if (toMinutes(tw.onHour, tw.onMin) > toMinutes(tw.offHour, tw.offMin)) {
        endDayAbs += 1440;
      }

      int diffStart = startDayAbs - nowAbs;
      int diffEnd = endDayAbs - nowAbs;

      if (diffStart >= 0 && diffStart < bestDiff) {
        bestDiff = diffStart;
        bestEvent = "LIGAR " + wdayName(d) + " " + twoDigits(tw.onHour) + ":" + twoDigits(tw.onMin);
      }

      if (diffEnd >= 0 && diffEnd < bestDiff) {
        bestDiff = diffEnd;
        bestEvent = "DESLIGAR " + wdayName(d) + " " + twoDigits(tw.offHour) + ":" + twoDigits(tw.offMin);
      }
    }
  }

  return bestEvent;
}

void checkIndustrialSchedule() {
  if (!autoMode || testMode || !browserTimeOk) return;

  if (millis() - lastScheduleCheck < 3000) return;
  lastScheduleCheck = millis();

  bool shouldBeOn = isIndustrialWindowNow();

  if (shouldBeOn != outputState) {
    setOutput(shouldBeOn, "AUTO_MULTI_WINDOW");
  }
}

// ========= TESTE 60/120 =========

unsigned long testStartMillis = 0;
bool testConfigured = false;
bool testOffExecuted = false;
bool testOnExecuted = false;


void setupTestScheduleMillis() {
  testMode = true;
  autoMode = true;

  testStartMillis = millis();
  testConfigured = true;
  testOffExecuted = false;
  testOnExecuted = false;

  addLog("[TEST] Agenda de teste configurada: OFF 60s / ON 120s");
}

String getTestNextEvent() {
  if (!testConfigured) return "Teste nao configurado";

  unsigned long elapsed = (millis() - testStartMillis) / 1000;

  if (!testOffExecuted) return "TESTE: DESLIGAR em " + String(max(0L, 60L - (long)elapsed)) + " s";
  if (!testOnExecuted) return "TESTE: LIGAR em " + String(max(0L, 120L - (long)elapsed)) + " s";

  return "Teste concluido";
}

void checkTestSchedule() {
  if (!autoMode || !testMode || !testConfigured) return;

  unsigned long elapsed = millis() - testStartMillis;

  if (!testOffExecuted && elapsed >= 60000) {
    setOutput(false, "AUTO_TEST_OFF_60S");
    testOffExecuted = true;
  }

  if (!testOnExecuted && elapsed >= 120000) {
    setOutput(true, "AUTO_TEST_ON_120S");
    testOnExecuted = true;
  }
}

// ========= BOTAO =========

void handleButton() {
  if (digitalRead(BTN_PIN) == LOW) {
    if (millis() - lastButtonTime > debounceDelay) {
      autoMode = false;
      testMode = false;
      setOutput(!outputState, "BOTAO_MANUAL");
      addLog("[MODE] MANUAL pelo botao.");
      lastButtonTime = millis();
    }
  }
}

// ========= HTML PRINCIPAL =========

String htmlPage() {
  String outputText = outputState ? "LIGADO" : "DESLIGADO";
  String modeText = autoMode ? "AUTO" : "MANUAL";
  String nextEvent = testMode ? getTestNextEvent() : getNextIndustrialEvent();

  String html = "";
  html += "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>SAPA IoT</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#101820;color:white;text-align:center;margin:0;padding:20px;}";
  html += ".brand-logo{max-width:360px;width:82%;height:auto;margin:4px auto 10px auto;display:block;}";
  html += ".card{background:#1b2733;border-radius:18px;padding:25px;margin:20px auto;max-width:760px;box-shadow:0 0 25px #000;}";
  html += "h1{color:#00d4ff;margin-bottom:5px;}";
  html += ".status{font-size:42px;font-weight:bold;margin:20px;}";
  html += ".on{color:#00ff66;}.off{color:#ff4444;}";
  html += ".mode{color:#ffaa00;font-size:24px;font-weight:bold;}";
  html += ".box{background:#111a22;border-radius:12px;padding:15px;margin:15px 0;}";
  html += ".info{font-size:18px;color:#ddd;margin:10px;}";
  html += ".clock{font-size:26px;color:#00d4ff;font-weight:bold;margin:10px;}";
  html += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:12px;margin:16px 0;}";
  html += ".kpi{background:#111a22;border-radius:14px;padding:18px;box-shadow:inset 0 0 0 1px rgba(255,255,255,.05);}";
  html += ".kpiTitle{font-size:12px;color:#aaa;text-transform:uppercase;letter-spacing:.7px;}";
  html += ".kpiValue{font-size:24px;font-weight:bold;color:#00d4ff;margin-top:7px;}";
  html += ".kpiWide{grid-column:span 2;min-height:90px;display:flex;flex-direction:column;justify-content:center;}";
  html += ".kpiGreen{color:#00ff66;}";
  html += ".kpiRed{color:#ff4444;}";
  html += ".kpiOrange{color:#ffaa00;}";
  html += ".kpiThermal{color:#00ffcc;}";
  html += "button{font-size:16px;padding:13px 18px;margin:7px;border:none;border-radius:12px;cursor:pointer;font-weight:bold;}";
  html += ".btnOn{background:#00aa44;color:white;}.btnOff{background:#cc2222;color:white;}";
  html += ".btnAuto{background:#ffaa00;color:black;}.btnManual{background:#555;color:white;}";
  html += ".btnTest{background:#00d4ff;color:#000;}.btnConfig{background:#8e44ad;color:white;}";
  html += ".btnLog{background:#2ecc71;color:#000;}.btnAbout{background:#34495e;color:white;}";
  html += ".watermark{position:fixed;right:10px;bottom:6px;color:rgba(255,255,255,.45);font-size:12px;text-align:right;}";
  html += "</style></head><body>";

  html += "<div class='card'>";
  html += "<img class='brand-logo' src='";
  html += SAPA_LOGO;
  html += "' alt='SAPA IoT Logo'>";
  html += "<h1>SAPA IoT</h1>";
  html += "<div style='color:#00d4ff;font-size:14px;font-weight:bold;'>v0.7.0 Sensor Ready</div>";
  html += "<div>Agenda Multi-Janela + Browser Time Sync</div>";

  html += "<div id='outputStatus' class='status ";
  html += outputState ? "on'>" : "off'>";
  html += outputText + "</div>";

  html += "<p>Modo: <span id='modeStatus' class='mode'>" + modeText + "</span></p>";

  html += "<div class='grid'>";

  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>Hora do dispositivo</div>";
  html += "<div id='liveClock' class='kpiValue'>Sincronizando...</div>";
  html += "</div>";

  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>Hora SAPA</div>";
  html += "<div id='espTime' class='kpiValue'>" + getTimeString() + "</div>";
  html += "</div>";

  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>Uptime</div>";
  html += "<div id='uptime' class='kpiValue'>" + getUptimeString() + "</div>";
  html += "</div>";

  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>Próximo evento</div>";
  html += "<div id='nextEvent' class='kpiValue'>" + nextEvent + "</div>";
  html += "</div>";

  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>Rede</div>";
  html += "<div class='kpiValue'>SAPA_IOT</div>";
  html += "</div>";

  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>IP</div>";
  html += "<div class='kpiValue'>192.168.4.1</div>";
  html += "</div>";
  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>Ligações</div>";
  html += "<div id='totalOn' class='kpiValue kpiGreen'>" + String(totalOnEvents) + "</div>";
  html += "</div>";

  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>Desligamentos</div>";
  html += "<div id='totalOff' class='kpiValue kpiRed'>" + String(totalOffEvents) + "</div>";
  html += "</div>";


  html += "<div class='kpi kpiWide'>";
  html += "<div class='kpiTitle'>Economia estimada</div>";
  html += "<div id='energySaved' class='kpiValue kpiGreen'>" + formatFloat1(getEstimatedSavedKwhYear()) + " kWh/ano</div>";
  html += "</div>";

  html += "<div class='kpi kpiWide'>";
  html += "<div class='kpiTitle'>Horas evitadas</div>";
  html += "<div id='hoursSaved' class='kpiValue'>" + formatFloat1(getEstimatedSavedHoursYear()) + " h/ano</div>";
  html += "</div>";


  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>Temp. ambiente</div>";
  html += "<div id='tempAmbient' class='kpiValue kpiThermal'>" + getTemperatureDisplay(tempAmbientC) + "</div>";
  html += "</div>";

  html += "<div class='kpi'>";
  html += "<div class='kpiTitle'>Temp. interna</div>";
  html += "<div id='tempInternal' class='kpiValue kpiThermal'>" + getTemperatureDisplay(tempInternalC) + "</div>";
  html += "</div>";

  html += "<div class='kpi kpiWide'>";
  html += "<div class='kpiTitle'>Último evento</div>";
  html += "<div id='lastEvent' class='kpiValue'>" + lastEventDashboard + "</div>";
  html += "</div>";

  html += "<div class='kpi kpiWide'>";
  html += "<div class='kpiTitle'>Último comando</div>";
  html += "<div id='lastSource' class='kpiValue kpiOrange'>" + lastCommandSource + "</div>";
  html += "</div>";
  html += "</div>";

  html += "<a href='/on'><button class='btnOn'>LIGAR</button></a>";
  html += "<a href='/off'><button class='btnOff'>DESLIGAR</button></a><br>";

  html += "<a href='/auto'><button class='btnAuto'>MODO AUTO</button></a>";
  html += "<a href='/manual'><button class='btnManual'>MODO MANUAL</button></a><br>";

  html += "<a href='/config'><button class='btnConfig'>CONFIGURAR AGENDA</button></a><br>";
  html += "<a href='/log'><button class='btnLog'>LOG / RELATORIO</button></a>";
  html += "<a href='/about'><button class='btnAbout'>SOBRE O SISTEMA</button></a><br>";

  html += "<a href='/test'><button class='btnTest'>TESTE 60/120s</button></a>";
  html += "<button class='btnTest' onclick='syncBrowserTime()'>SINCRONIZAR COM ESTE DISPOSITIVO</button>";

  html += "</div>";

  html += "<script>";
  html += "function pad(n){return n<10?'0'+n:n;}";
  html += "function updateClock(){var d=new Date();var dias=['Domingo','Segunda','Terca','Quarta','Quinta','Sexta','Sabado'];document.getElementById('liveClock').innerHTML=dias[d.getDay()]+' '+pad(d.getHours())+':'+pad(d.getMinutes())+':'+pad(d.getSeconds())+' (PC/Celular)';}";
  html += "function syncBrowserTime(){var d=new Date();fetch('/browsertime?wday='+d.getDay()+'&h='+d.getHours()+'&m='+d.getMinutes()+'&s='+d.getSeconds()).then(()=>updateStatus());}";
  html += "function updateStatus(){fetch('/status').then(r=>r.json()).then(data=>{let s=document.getElementById('outputStatus');s.innerHTML=data.output;s.className='status '+(data.output=='LIGADO'?'on':'off');document.getElementById('modeStatus').innerHTML=data.mode;document.getElementById('espTime').innerHTML=data.time;document.getElementById('uptime').innerHTML=data.uptime;document.getElementById('nextEvent').innerHTML=data.next;document.getElementById('totalOn').innerHTML=data.totalOn;document.getElementById('totalOff').innerHTML=data.totalOff;document.getElementById('lastEvent').innerHTML=data.lastEvent;document.getElementById('lastSource').innerHTML=data.lastSource;document.getElementById('energySaved').innerHTML=data.energySaved;document.getElementById('hoursSaved').innerHTML=data.hoursSaved;document.getElementById('tempAmbient').innerHTML=data.tempAmbient;document.getElementById('tempInternal').innerHTML=data.tempInternal;});}";
  html += "setInterval(updateClock,1000);setInterval(updateStatus,1000);updateClock();";
  html += "if(!sessionStorage.getItem('sapa_time_synced')){setTimeout(()=>{syncBrowserTime();sessionStorage.setItem('sapa_time_synced','1');},800);}";
  html += "</script>";

  html += "<div class='watermark'>Developed by Andre Gama de Matos - Software Engineer<br>SAPA IoT v0.7.0 Sensor Ready</div>";

  html += "</body></html>";
  return html;
}

// ========= HTML CONFIG =========

String configPage() {
  String html = "";
  html += "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Configurar Agenda Multi-Janela</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#101820;color:white;text-align:center;padding:20px;}";
  html += ".card{background:#1b2733;border-radius:18px;padding:25px;margin:18px auto;max-width:980px;box-shadow:0 0 25px #000;}";
  html += "h1{color:#00d4ff;} h2{color:#00d4ff;margin-bottom:10px;}";
  html += "table{width:100%;border-collapse:collapse;margin-top:10px;}";
  html += "td,th{padding:8px;border-bottom:1px solid #34495e;}";
  html += "input{font-size:16px;padding:7px;border-radius:8px;border:none;text-align:center;}";
  html += "button{font-size:17px;padding:14px 22px;margin:12px;border:none;border-radius:12px;font-weight:bold;cursor:pointer;}";
  html += ".save{background:#00aa44;color:white;}.back{background:#555;color:white;}";
  html += ".hint{font-size:14px;color:#ccc;}";
  html += ".watermark{position:fixed;right:10px;bottom:6px;color:rgba(255,255,255,.45);font-size:12px;text-align:right;}";
  html += "</style></head><body>";
  html += "<h1>Configurar Agenda Multi-Janela</h1>";
  html += "<p class='hint'>Cada dia pode ter ate 8 janelas de LIGAR/DESLIGAR.</p>";

  if (server.hasArg("saved")) {
    html += "<div style='background:#00aa44;color:white;padding:14px;border-radius:12px;margin:15px auto;max-width:850px;font-weight:bold;'>";
    html += "AGENDA SALVA COM SUCESSO ✓";
    html += "</div>";
  }

  html += "<form id='scheduleForm' action='/saveconfig' method='GET'>";

  for (int d = 0; d < 7; d++) {
    html += "<div class='card'>";
    html += "<h2>" + wdayName(d) + "</h2>";
    html += "<table><tr><th>Ativo</th><th>Janela</th><th>Ligar</th><th>Desligar</th></tr>";

    for (int w = 0; w < MAX_WINDOWS_PER_DAY; w++) {
      TimeWindow tw = scheduleTable[d][w];

      String base = "d" + String(d) + "w" + String(w);

      html += "<tr>";
      html += "<td><input type='checkbox' name='" + base + "en' ";
      if (tw.enabled) html += "checked";
      html += "></td>";

      html += "<td>" + String(w + 1) + "</td>";

      html += "<td>";
      html += "<input type='number' name='" + base + "onh' min='0' max='23' value='" + String(tw.onHour) + "' style='width:55px;'> : ";
      html += "<input type='number' name='" + base + "onm' min='0' max='59' value='" + String(tw.onMin) + "' style='width:55px;'>";
      html += "</td>";

      html += "<td>";
      html += "<input type='number' name='" + base + "offh' min='0' max='23' value='" + String(tw.offHour) + "' style='width:55px;'> : ";
      html += "<input type='number' name='" + base + "offm' min='0' max='59' value='" + String(tw.offMin) + "' style='width:55px;'>";
      html += "</td>";

      html += "</tr>";
    }

    html += "</table>";
    html += "</div>";
  }

  html += "<button id='saveBtn' class='save' type='submit'>SALVAR AGENDA</button>";
  html += "</form>";
  html += "<a href='/'><button class='back'>VOLTAR</button></a>";

  html += "<script>";
  html += "const form=document.getElementById('scheduleForm');";
  html += "form.addEventListener('submit',()=>{let btn=document.getElementById('saveBtn');btn.innerHTML='SALVANDO...';btn.style.background='#3498db';});";
  html += "</script>";

  html += "<div class='watermark'>Developed by Andre Gama de Matos - Software Engineer<br>SAPA IoT v0.5.3</div>";
  html += "</body></html>";
  return html;
}

// ========= PAGINAS DE LOG E SOBRE =========

String logPage() {
  String html = "";
  html += "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>SAPA IoT - Log</title>";
  html += "<style>body{font-family:Arial;background:#101820;color:white;text-align:center;padding:20px;}";
  html += ".card{background:#1b2733;border-radius:18px;padding:25px;margin:18px auto;max-width:980px;box-shadow:0 0 25px #000;}";
  html += "h1{color:#00d4ff}.log{text-align:left;font-family:monospace;background:#071018;border-radius:12px;padding:16px;line-height:1.55;overflow:auto;}";
  html += "button{font-size:16px;padding:12px 18px;margin:8px;border:none;border-radius:12px;font-weight:bold;cursor:pointer;background:#555;color:white;}";
  html += ".watermark{position:fixed;right:10px;bottom:6px;color:rgba(255,255,255,.45);font-size:12px;text-align:right;}";
  html += "</style></head><body>";
  html += "<div class='card'><h1>SAPA IoT - LOG / RELATORIO</h1>";
  html += "<p>Uptime atual: " + getUptimeString() + "</p>";
  html += "<p>Estado: <b>" + String(outputState ? "LIGADO" : "DESLIGADO") + "</b> | Modo: <b>" + String(autoMode ? "AUTO" : "MANUAL") + "</b></p>";
  html += "<p>Rede: SAPA_IOT | IP: 192.168.4.1</p>";
  html += "<p>Log CSV persistente: <b>/events.csv</b> ";
  if (littleFsReady && LittleFS.exists(LOG_FILE)) {
    File lf = LittleFS.open(LOG_FILE, FILE_READ);
    html += "(" + String(lf.size()) + " bytes)";
    lf.close();
  } else {
    html += "(indisponivel)";
  }
  html += "</p>";
  html += "<a href='/downloadlog'><button style='background:#00aa44;color:white;'>DOWNLOAD CSV LOG</button></a>";
  html += "<a href='/clearlog' onclick=\"return confirm('Limpar log CSV?');\"><button style='background:#cc2222;color:white;'>LIMPAR LOG</button></a>";
  html += "<div class='log'>";
  if (logCount == 0) {
    html += "Nenhum evento registrado ainda.<br>";
  } else {
    int start = (logIndex - logCount + MAX_LOGS) % MAX_LOGS;
    for (int i = 0; i < logCount; i++) {
      int idx = (start + i) % MAX_LOGS;
      html += eventLogs[idx] + "<br>";
    }
  }
  html += "</div><br><a href='/'><button>VOLTAR</button></a></div>";
  html += "<div class='watermark'>Developed by Andre Gama de Matos - Software Engineer<br>SAPA IoT v0.5.3</div>";
  html += "</body></html>";
  return html;
}

String aboutPage() {
  String html = "";
  html += "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>SAPA IoT - Sobre</title>";
  html += "<style>body{font-family:Arial;background:#101820;color:white;text-align:center;padding:20px;}";
  html += ".card{background:#1b2733;border-radius:18px;padding:25px;margin:18px auto;max-width:880px;box-shadow:0 0 25px #000;}";
  html += "h1{color:#00d4ff} h2{color:#00d4ff}.info{text-align:left;line-height:1.7;font-size:17px;}";
  html += "button{font-size:16px;padding:12px 18px;margin:8px;border:none;border-radius:12px;font-weight:bold;cursor:pointer;background:#555;color:white;}";
  html += ".watermark{position:fixed;right:10px;bottom:6px;color:rgba(255,255,255,.45);font-size:12px;text-align:right;}";
  html += "</style></head><body>";
  html += "<div class='card'><h1>SAPA IoT</h1><h2>Sistema Autonomo de Potencia para Amplificadores</h2>";
  html += "<div class='info'>";
  html += "<p><b>Versao:</b> ";
  html += FW_VERSION;
  html += "</p>";

  html += "<p><b>Release:</b> ";
  html += RELEASE_DATE;
  html += "</p>";
  html += "<p><b>Finalidade:</b> controle automatico de liga/desliga de amplificador industrial por agenda multi-janela.</p>";
  html += "<p><b>Tecnologias:</b><br>ESP32<br>WebServer local<br>Access Point Standalone<br>Agenda Multi-Window<br>Controle SSR/Contactora<br>Log interno circular<br>LittleFS CSV Logging<br>Download de log via Web<br>Economia estimada<br>Sensor Ready para monitoramento termico</p>";
  html += "<p><b>Hardware:</b><br>SSR/Contactora, botao fisico, LEDs de status e futura expansao com RTC DS3231, sensor de temperatura ambiente e sensor de temperatura interna/exaustao.</p>";
  html += "<p><b>Desenvolvido por:</b><br>Andre Gama de Matos - Software Engineer</p>";
  html += "</div><br><a href='/'><button>VOLTAR</button></a></div>";
  html += "<div class='watermark'>Developed by Andre Gama de Matos - Software Engineer<br>SAPA IoT v0.5.3</div>";
  html += "</body></html>";
  return html;
}

// ========= ROTAS =========

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleConfig() {
  server.send(200, "text/html", configPage());
}

void handleLog() {
  server.send(200, "text/html", logPage());
}

void handleAbout() {
  server.send(200, "text/html", aboutPage());
}

void handleDownloadLog() {
  if (!littleFsReady || !LittleFS.exists(LOG_FILE)) {
    server.send(404, "text/plain", "Log CSV nao encontrado.");
    return;
  }

  File f = LittleFS.open(LOG_FILE, FILE_READ);
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=SAPA_IoT_events_v0_5_2.csv");
  server.streamFile(f, "text/csv");
  f.close();
}

void handleClearLog() {
  if (littleFsReady) {
    LittleFS.remove(LOG_FILE);
    ensureCsvHeader();
    addLog("[LOG] CSV limpo pelo usuario.");
  }
  server.sendHeader("Location", "/log");
  server.send(303);
}

void handleSaveConfig() {
  for (int d = 0; d < 7; d++) {
    for (int w = 0; w < MAX_WINDOWS_PER_DAY; w++) {
      String base = "d" + String(d) + "w" + String(w);

      scheduleTable[d][w].enabled = server.hasArg(base + "en");

      if (server.hasArg(base + "onh")) scheduleTable[d][w].onHour = server.arg(base + "onh").toInt();
      if (server.hasArg(base + "onm")) scheduleTable[d][w].onMin = server.arg(base + "onm").toInt();

      if (server.hasArg(base + "offh")) scheduleTable[d][w].offHour = server.arg(base + "offh").toInt();
      if (server.hasArg(base + "offm")) scheduleTable[d][w].offMin = server.arg(base + "offm").toInt();

      scheduleTable[d][w].onHour = constrain(scheduleTable[d][w].onHour, 0, 23);
      scheduleTable[d][w].onMin = constrain(scheduleTable[d][w].onMin, 0, 59);
      scheduleTable[d][w].offHour = constrain(scheduleTable[d][w].offHour, 0, 23);
      scheduleTable[d][w].offMin = constrain(scheduleTable[d][w].offMin, 0, 59);
    }
  }

  addLog("[CONFIG] Agenda multi-janela atualizada.");

  if (autoMode && !testMode) {
    checkIndustrialSchedule();
  }

  server.sendHeader("Location", "/config?saved=1");
  server.send(303);
}

void handleStatus() {
  String json = "{";
  json += "\"output\":\"" + String(outputState ? "LIGADO" : "DESLIGADO") + "\",";
  json += "\"mode\":\"" + String(autoMode ? "AUTO" : "MANUAL") + "\",";
  json += "\"time\":\"" + getTimeString() + "\",";
  json += "\"uptime\":\"" + getUptimeString() + "\",";
  json += "\"next\":\"" + String(testMode ? getTestNextEvent() : getNextIndustrialEvent()) + "\",";
  json += "\"totalOn\":\"" + String(totalOnEvents) + "\",";
  json += "\"totalOff\":\"" + String(totalOffEvents) + "\",";
  json += "\"lastEvent\":\"" + lastEventDashboard + "\",";
  json += "\"lastSource\":\"" + lastCommandSource + "\",";
  json += "\"energySaved\":\"" + formatFloat1(getEstimatedSavedKwhYear()) + " kWh/ano\",";
  json += "\"hoursSaved\":\"" + formatFloat1(getEstimatedSavedHoursYear()) + " h/ano\",";
  json += "\"tempAmbient\":\"" + getTemperatureDisplay(tempAmbientC) + "\",";
  json += "\"tempInternal\":\"" + getTemperatureDisplay(tempInternalC) + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void handleBrowserTime() {
  if (server.hasArg("wday") && server.hasArg("h") && server.hasArg("m") && server.hasArg("s")) {
    browserWday = server.arg("wday").toInt();
    browserHour = server.arg("h").toInt();
    browserMinute = server.arg("m").toInt();
    browserSecond = server.arg("s").toInt();

    browserBaseMillis = millis();
    browserTimeOk = true;

    addLog("[BROWSER_TIME] " + getTimeString());

    if (autoMode && !testMode) checkIndustrialSchedule();
  }

  server.send(200, "text/plain", "OK");
}

void handleOn() {
  autoMode = false;
  testMode = false;
  setOutput(true, "WEB_MANUAL_ON");
  addLog("[MODE] MANUAL ativado.");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  autoMode = false;
  testMode = false;
  setOutput(false, "WEB_MANUAL_OFF");
  addLog("[MODE] MANUAL ativado.");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleAuto() {
  autoMode = true;
  testMode = false;
  addLog("[MODE] AUTO ativado.");
  checkIndustrialSchedule();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleManual() {
  autoMode = false;
  testMode = false;
  addLog("[MODE] MANUAL ativado.");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleTest() {
  setupTestScheduleMillis();
  addLog("[MODE] TESTE 60/120s ativado.");
  server.sendHeader("Location", "/");
  server.send(303);
}

// ========= SETUP =========

void setup() {
  Serial.begin(115200);

  pinMode(SSR_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);

  digitalWrite(LED_YELLOW, HIGH);

  loadDefaultMultiWindowSchedule();

  outputState = false;
  applyOutput();

  Serial.println();

  littleFsReady = LittleFS.begin(true);
  if (littleFsReady) {
    ensureCsvHeader();
    Serial.println("[LittleFS] Sistema de arquivos iniciado.");
  } else {
    Serial.println("[LittleFS] ERRO ao iniciar sistema de arquivos.");
  }

  addLog("[BOOT] SAPA IoT v0.6.1 - Dashboard + Estatísticas");
  addLog("[OUTPUT] OFF | Fonte: BOOT_SAFE_OFF");

  // ========= MODO STANDALONE AP =========
  // O ESP32 nao depende mais de notebook, hotspot, roteador ou internet.
  WiFi.mode(WIFI_AP);

  if (!WiFi.softAPConfig(ap_IP, ap_gateway, ap_subnet)) {
    Serial.println("[WIFI_AP] Falha ao configurar IP do Access Point");
  }

  bool apOk = WiFi.softAP(ap_ssid, ap_password);

  if (apOk) {
    addLog("[WIFI_AP] Rede criada com sucesso.");
    Serial.print("[WIFI_AP] SSID: ");
    Serial.println(ap_ssid);
    Serial.print("[WIFI_AP] Senha: ");
    Serial.println(ap_password);
    Serial.print("[WIFI_AP] Acesse: http://");
    Serial.println(WiFi.softAPIP());
  } else {
    addLog("[WIFI_AP] ERRO ao criar rede Wi-Fi.");
  }

  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/log", handleLog);
  server.on("/about", handleAbout);
  server.on("/downloadlog", handleDownloadLog);
  server.on("/clearlog", handleClearLog);
  server.on("/saveconfig", handleSaveConfig);
  server.on("/status", handleStatus);
  server.on("/browsertime", handleBrowserTime);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/auto", handleAuto);
  server.on("/manual", handleManual);
  server.on("/test", handleTest);

  server.begin();
  addLog("[WEB] Servidor iniciado.");
}

// ========= LOOP =========

void loop() {
  server.handleClient();
  handleButton();
  checkTestSchedule();
  checkIndustrialSchedule();
}
