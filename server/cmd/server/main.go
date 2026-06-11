// cmd/server/main.go — Mock Server，Phase 1

package main

import (
	"crypto/hmac"
	"crypto/sha256"
	"encoding/base64"
	"encoding/json"
	"log"
	"net/http"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"

	"github.com/gorilla/websocket"
)

var jwtSecret = []byte("yunrong-mock-secret")

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool { return true },
}

func main() {
	mux := http.NewServeMux()
	mux.HandleFunc("GET /health", healthHandler)
	mux.HandleFunc("POST /api/v1/auth/login", loginHandler)
	mux.HandleFunc("/ws", wsHandler)

	addr := ":8080"
	log.Printf("Mock Server listening on %s", addr)

	go func() {
		if err := http.ListenAndServe(addr, mux); err != nil {
			log.Fatalf("server error: %v", err)
		}
	}()

	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	log.Println("Mock Server shutting down...")
}

// ===== handlers =====

func healthHandler(w http.ResponseWriter, r *http.Request) {
	writeJSON(w, http.StatusOK, map[string]string{"status": "ok"})
}

func loginHandler(w http.ResponseWriter, r *http.Request) {
	var req struct {
		Username string `json:"username"`
		Password string `json:"password"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		writeJSON(w, http.StatusBadRequest, map[string]string{"error": "invalid json"})
		return
	}

	users := map[string]struct {
		password string
		userID   int64
		name     string
	}{
		"admin":    {"123456", 1001, "管理员"},
		"zhangsan": {"123456", 1002, "张三"},
		"lisi":     {"123456", 1003, "李四"},
	}

	u, ok := users[req.Username]
	if !ok || u.password != req.Password {
		writeJSON(w, http.StatusUnauthorized, map[string]string{"error": "invalid credentials"})
		return
	}

	token, err := makeJWT(u.userID, req.Username)
	if err != nil {
		writeJSON(w, http.StatusInternalServerError, map[string]string{"error": "token generation failed"})
		return
	}

	writeJSON(w, http.StatusOK, map[string]any{
		"code":    0,
		"message": "success",
		"data": map[string]any{
			"user_id":       u.userID,
			"display_name":  u.name,
			"access_token":  token,
			"refresh_token": token,
			"expires_in":    7200,
			"ws_url":        "ws://localhost:8080/ws",
		},
	})
}

func wsHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("ws upgrade failed: %v", err)
		return
	}
	defer conn.Close()

	log.Printf("ws client connected from %s", r.RemoteAddr)

	for {
		_, msg, err := conn.ReadMessage()
		if err != nil {
			log.Printf("ws client disconnected: %v", err)
			return
		}

		var req map[string]any
		if err := json.Unmarshal(msg, &req); err != nil {
			log.Printf("ws non-JSON received: %s", string(msg))
			continue
		}

		log.Printf("ws received: %s", string(msg))

		// 回声：ping → pong，其他 → echo
		reqType, _ := req["type"].(string)
		var resp map[string]any
		if reqType == "ping" {
			resp = map[string]any{"type": "pong"}
		} else {
			resp = map[string]any{"type": "echo", "payload": req}
		}

		respBytes, _ := json.Marshal(resp)
		conn.WriteMessage(websocket.TextMessage, respBytes)
	}
}

// ===== JWT（标准库手写，Mock 专用）=====

func makeJWT(userID int64, username string) (string, error) {
	header := base64.RawURLEncoding.EncodeToString(
		[]byte(`{"alg":"HS256","typ":"JWT"}`))

	payloadBytes, _ := json.Marshal(map[string]any{
		"user_id":  userID,
		"username": username,
		"exp":      time.Now().Add(2 * time.Hour).Unix(),
		"iat":      time.Now().Unix(),
	})
	payload := base64.RawURLEncoding.EncodeToString(payloadBytes)

	unsigned := header + "." + payload
	mac := hmac.New(sha256.New, jwtSecret)
	mac.Write([]byte(unsigned))
	sig := base64.RawURLEncoding.EncodeToString(mac.Sum(nil))

	return unsigned + "." + sig, nil
}

// ===== helpers =====

func writeJSON(w http.ResponseWriter, status int, v any) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(status)
	json.NewEncoder(w).Encode(v)
}

func init() {
	tok, _ := makeJWT(1001, "admin")
	log.Println("Admin test token (valid 2h):")
	log.Println("  " + tok)
	log.Println(strings.Repeat("-", 60))
}
