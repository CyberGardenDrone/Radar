package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"github.com/tarm/serial"
	"log"
	"net/http"
)

var comPortBuffer []string

type comPortLine struct {
	Line string `json:"line"`
}

func apiGetHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != "GET" {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}
	lines := make([]comPortLine, len(comPortBuffer))
	for i, line := range comPortBuffer {
		lines[i] = comPortLine{Line: line}
	}

	jsonLines, err := json.Marshal(lines)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.Write(jsonLines)

	comPortBuffer = nil
}

func readFromSerial(stream *serial.Port) {
	scanner := bufio.NewScanner(stream)
	for scanner.Scan() {
		line := scanner.Text()
		fmt.Println(line)
		comPortBuffer = append(comPortBuffer, line)
	}
	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}
}

func main() {
	c := &serial.Config{Name: "COM6", Baud: 9600}
	stream, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}

	http.HandleFunc("/api/get/", apiGetHandler)
	go func() { log.Fatal(http.ListenAndServe(":11130", nil)) }() // Запускаем HTTP-сервер в отдельном потоке

	go readFromSerial(stream) // Запускаем чтение из COM-порта в отдельном потоке

	select {} // Блокируем основной поток, чтобы программа не завершалась
}
