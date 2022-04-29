import urllib3
flag = "bctf{sorry_no_nfts_only_flags}"

def send(flag):
    http = urllib3.PoolManager()
    for c in flag:
        http.request("GET", f"http://localhost:7878/{c}")
    http.request("GET", "http://localhost:7878/crawl")

send(flag)
