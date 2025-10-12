package encrypt

import (
	"celer/pkgs/color"
	"os"
	"path/filepath"

	"golang.org/x/crypto/bcrypt"
)

func Encode(content string) ([]byte, error) {
	hashed, err := bcrypt.GenerateFromPassword([]byte(content), bcrypt.DefaultCost)
	if err != nil {
		return nil, err
	}
	return hashed, nil
}

func CheckToken(tokenDir, encoded string) bool {
	bytes, err := os.ReadFile(filepath.Join(tokenDir, "token"))
	if err != nil {
		color.Printf(color.Yellow, "failed to read token:\n %s", err)
		return false
	}
	return bcrypt.CompareHashAndPassword(bytes, []byte(encoded)) == nil
}
