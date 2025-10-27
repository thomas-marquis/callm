package srvprobe

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"sync"

	"github.com/gorilla/mux"
)

type RequestData struct {
	Message string      `json:"message"`
	Matrix  [][]float64 `json:"matrix"`
}

type Message struct {
	Data    interface{} `json:"matrix"`
	Message string      `json:"label"`
}

type client struct {
	ID     int
	Events chan Message
}

type server struct {
	clients    map[int]client
	register   chan client
	unregister chan client
	broadcast  chan Message
	mutex      sync.Mutex
	idCounter  int
}

func newServer() *server {
	return &server{
		clients:    make(map[int]client),
		register:   make(chan client),
		unregister: make(chan client),
		broadcast:  make(chan Message),
		idCounter:  0,
	}
}

func (s *server) HandleSSEConnection(w http.ResponseWriter, r *http.Request) {
	flusher, ok := w.(http.Flusher)
	if !ok {
		http.Error(w, "Streaming unsupported!", http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "text/event-stream")
	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Connection", "keep-alive")
	w.Header().Set("Access-Control-Allow-Origin", "*")

	client := client{
		ID:     s.idCounter,
		Events: make(chan Message),
	}
	s.idCounter++

	s.register <- client
	defer func() {
		s.unregister <- client
		close(client.Events)
	}()

	for {
		select {
		case msg := <-client.Events:
			fmt.Printf("Sending message: %v\n", msg)
			data, err := json.Marshal(msg)
			if err != nil {
				log.Printf("Error marshalling message: %v", err)
				continue
			}
			fmt.Fprintf(w, "data: %s\n\n", data)
			flusher.Flush()
		case <-r.Context().Done():
			fmt.Println("Client closed connection")
			return
		}
	}
}

func (s *server) HandlePostMatrix(w http.ResponseWriter, r *http.Request) {
	log.Println("Received POST /data request")
	var requestData RequestData
	err := json.NewDecoder(r.Body).Decode(&requestData)
	if err != nil {
		log.Printf("Error decoding request body: %v", err)
		http.Error(w, "Failed to decode request body", http.StatusBadRequest)
		return
	}
	defer r.Body.Close()

	s.SendMessage(requestData.Matrix, requestData.Message)

	w.WriteHeader(http.StatusCreated)
	w.Write([]byte("Data received successfully"))
}

func (s *server) SendMessage(data interface{}, message string) {
	s.broadcast <- Message{Data: data, Message: message}
}

func (s *server) Start() {
	go func() {
		for {
			select {
			case client := <-s.register:
				s.mutex.Lock()
				s.clients[client.ID] = client
				s.mutex.Unlock()
				log.Printf("Client %d registered", client.ID)
			case client := <-s.unregister:
				s.mutex.Lock()
				delete(s.clients, client.ID)
				s.mutex.Unlock()
				log.Printf("Client %d unregistered", client.ID)
			case msg := <-s.broadcast:
				s.mutex.Lock()
				for _, client := range s.clients {
					client.Events <- msg
				}
				s.mutex.Unlock()
			}
		}
	}()
}

func Run() {
	sseServer := newServer()
	sseServer.Start()

	router := mux.NewRouter()
	router.HandleFunc("/events", sseServer.HandleSSEConnection).Methods("GET")
	router.HandleFunc("/data", sseServer.HandlePostMatrix).Methods("POST")

	log.Println("Starting server on :8081")
	log.Fatal(http.ListenAndServe(":8081", router))
}
