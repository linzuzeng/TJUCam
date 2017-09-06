package main

import (
	"fmt"
	"net/http"
	"os"
	"os/exec"
	"path"
)

func main() {
	ex, err2 := os.Executable()
	if err2 != nil {
		panic(err2)
	}
	exPath := path.Dir(ex)
	os.Chdir(exPath)
	fmt.Printf("%s\n", exPath)
	http.Handle("/", http.FileServer(http.Dir("./")))

	http.HandleFunc("/exec", func(w http.ResponseWriter, r *http.Request) {

		cmd := r.URL.Query().Get("cmd")
		args := r.URL.Query()["arg"]

		w.Write(runCmd(cmd, args))
	})


	http.HandleFunc("/systemctl", func(w http.ResponseWriter, r *http.Request) {
		cmd := r.URL.Query().Get("cmd")

		w.Write(runShell("systemctl "+cmd))
	})

	var port = ":80"

	fmt.Printf("Server listening - http://%s%s", "[::0]", port)

	err := http.ListenAndServe(port, nil)

	if err != nil {
		fmt.Printf(err.Error())
	}

}

func runCmd(cmd string, args []string) []byte {
	
	fmt.Printf("executing with %s %s", cmd , args)
	out, err := exec.Command(cmd, args...).CombinedOutput()
	if err != nil {
		return []byte(fmt.Sprintf("<html><pre>%v</pre><pre>%s</pre></html>", err,out))
	}
	return []byte(fmt.Sprintf("<html><pre>%s</pre></html>", out))
}
func runShell(cmd string) []byte {
	fmt.Printf("executing with %s \n", cmd )
	out, err := exec.Command("bash" ,"-c", cmd).CombinedOutput()
	if err != nil {
		return []byte(fmt.Sprintf("<html><pre>%v</pre><pre>%s</pre></html>", err,out))
	}
	return []byte(fmt.Sprintf("<html><pre>done.</pre><pre>%s</pre></html>", out))
}

